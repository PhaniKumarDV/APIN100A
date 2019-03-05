/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/****< ipsapi.h >**************************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  IPSAPI - Qualcomm Technologies Bluetooth Indoor Positioning Service (GATT */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/29/15  R. McCord      Initial Creation.                               */
/******************************************************************************/
#ifndef __IPSAPIH__
#define __IPSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "IPSTypes.h"      /* Indoor Positioning Service Types/Constants.     */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define IPS_ERROR_INVALID_PARAMETER                      (-1000)
#define IPS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define IPS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define IPS_ERROR_INSUFFICIENT_BUFFER_SPACE              (-1003)
#define IPS_ERROR_SERVICE_ALREADY_REGISTERED             (-1004)
#define IPS_ERROR_INVALID_INSTANCE_ID                    (-1005)
#define IPS_ERROR_MALFORMATTED_DATA                      (-1006)

#define IPS_ERROR_FEATURE_NOT_SUPPORTED                  (-1007)
#define IPS_ERROR_INVALID_CONFIGURATION                  (-1008)
#define IPS_ERROR_INVALID_STRING_LENGTH                  (-1009)
#define IPS_ERROR_READ_TX_POWER_FAILED                   (-1010)
#define IPS_ERROR_LOCATION_NAME_NOT_CONFIGURED           (-1011)
#define IPS_ERROR_NO_PENDING_AUTHORIZATION_REQUEST       (-1012)

   /* IPS Common API Structures, Enums, and Constants.                  */

   /* The following enumeration defines the characteristic types.       */
   /* * NOTE * The ictExtendedProperties is a Characteristic descriptor.*/
typedef enum
{
   ictIndoorPositioningConfiguration,
   ictLatitude,
   ictLongitude,
   ictLocalNorthCoordinate,
   ictLocalEastCoordinate,
   ictFloorNumber,
   ictAltitude,
   ictUncertainty,
   ictLocationName,
   ictExtendedProperties
} IPS_Characteristic_Type_t;

   /* IPS Server API Structures, Enums, and Constants.                  */

   /* The following defines the flags that may be used to initialize the*/
   /* service with optional IPS Characteristics.  These flags make up   */
   /* the bit mask for the IPS_Characteristic_Flags field of the        */
   /* IPS_Initialize_Data_t structure for the IPS_Initialize_XXX() API's*/
   /* used to register IPS.                                             */
#define IPS_CHARACTERISTIC_FLAGS_LOCAL_COORDINATES                 (0x01)
#define IPS_CHARACTERISTIC_FLAGS_FLOOR_NUMBER                      (0x02)
#define IPS_CHARACTERISTIC_FLAGS_ALTITUDE                          (0x04)
#define IPS_CHARACTERISTIC_FLAGS_UNCERTAINTY                       (0x08)
#define IPS_CHARACTERISTIC_FLAGS_LOCATION_NAME                     (0x10)

   /* The following defines the optional IPS Characteristic properties  */
   /* that may be set for IPS Characteristics.  These flags make up the */
   /* bit mask for the IPS_Characteristic_Property_Flags parameter of   */
   /* the IPS_Initialize_Data_t structure for the IPS_Initialize_XXX()  */
   /* API's used to register IPS.                                       */
   /* * NOTE * The IPS_CHARACTERISTIC_PROPERTY_FLAGS_XXX Flags will     */
   /*          enable the IPS Characteristic property for all supported */
   /*          characteristics (This includes any optional IPS          */
   /*          Characteristic defined by the IPS_Characteristic_Flags   */
   /*          field of the IPS_Initialize_Data_t structure).           */
   /* * NOTE * IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE */
   /*          and IPS_CHARACTERISTIC_PROPERTY_FLAGS_RELIABLE_WRITE can */
   /*          only be included if                                      */
   /*          IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE is set.  If      */
   /*          IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE is NOT SET then  */
   /*          the other Flags will be IGNORED if set.                  */
   /* * NOTE * If IPS_CHARACTERISTIC_PROPERTY_FLAGS_RELIABLE_WRITE is   */
   /*          set then an Extended Properties descriptor will be       */
   /*          present for each support characteristic.  The 'reliable  */
   /*          write bit' will also be automaticlly set since it is     */
   /*          required to be set for reliable writes.                  */
#define IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE                   (0x01)
#define IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE  (0x02)
#define IPS_CHARACTERISTIC_PROPERTY_FLAGS_RELIABLE_WRITE          (0x04)

   /* The following defines the initialize information that is passed to*/
   /* the IPS_Initialize_XXX() API's to configure optional service      */
   /* features.                                                         */
   /* * NOTE * The Enable_Authorized_Device_Write field configures the  */
   /*          IPS Server to only allow authorized IPS Clients to write */
   /*          IPS Characteristics.  In order to use this feature the   */
   /*          IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE flag MUST be set */
   /*          in the IPS_Characteristic_Properties_Flags field.  This  */
   /*          feature will also affect the                             */
   /*          IPS_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE */
   /*          and IPS_CHARACTERISTIC_PROPERTY_FLAGS_RELIABLE_WRITE     */
   /*          flags if they are also specified in the                  */
   /*          IPS_Characteristic_Properties_Flags field.               */
   /* * NOTE * If Enable_Authorized_Device_Write is TRUE then an        */
   /*          etIPS_Server_Authorization_Request event will be         */
   /*          dispatched when a request to write an IPS Characteristic */
   /*          value or prepare an IPS Characteristic to be written     */
   /*          (reliabled write or write long) is received.  If the IPS */
   /*          Client that made the request is authorized via the       */
   /*          IPS_Authorization_Request_Response() function, then the  */
   /*          etIPS_Server_Characteristic_Updated event will be        */
   /*          dispatched when the IPS Characteristic(s) have been      */
   /*          written.                                                 */
typedef struct _tagIPS_Initialize_Data_t
{
   unsigned int  IPS_Characteristic_Flags;
   unsigned int  IPS_Characteristic_Property_Flags;
   Boolean_t     Enable_Authorized_Device_Write;
} IPS_Initialize_Data_t;

#define IPS_INITIALIZE_DATA_SIZE                         (sizeof(IPS_Initialize_Data_t))

   /* The following defines the flags that may be used indicate the     */
   /* characteristics that were read or updated.                        */
#define IPS_CHARACTERISTIC_INDOOR_POSITIONING_CONFIG   (0x0001)
#define IPS_CHARACTERISTIC_LATITUDE                    (0x0002)
#define IPS_CHARACTERISTIC_LONGITUDE                   (0x0004)
#define IPS_CHARACTERISTIC_LOCAL_NORTH_COORDINATE      (0x0008)
#define IPS_CHARACTERISTIC_LOCAL_EAST_COORDINATE       (0x0010)
#define IPS_CHARACTERISTIC_FLOOR_NUMBER                (0x0020)
#define IPS_CHARACTERISTIC_ALTITUDE                    (0x0040)
#define IPS_CHARACTERISTIC_UNCERTAINTY                 (0x0080)
#define IPS_CHARACTERISTIC_LOCATION_NAME               (0x0100)

   /* The following enumeration covers all the events generated by the  */
   /* IPS Service. These are used to determine the type of each event   */
   /* generated and to ensure the proper union element is accessed for  */
   /* the IPS_Event_Data_t structure.                                   */
typedef enum
{
   etIPS_Server_Characteristic_Read,
   etIPS_Server_Characteristic_Updated,
   etIPS_Server_Authorization_Request
} IPS_Event_Type_t;

   /* The following is dispatched to a IPS Server to inform the         */
   /* application that an IPS Client has read a characteristic.  The    */
   /* type of characteristic will be identified by the Type field.      */
   /* * NOTE * The application may not care if an IPS Characteristic has*/
   /*          been read by the IPS Client, however this event is passed*/
   /*          to inform the application.  This event is mainly         */
   /*          informative and the application may choose to ignore it. */
typedef struct _tagIPS_Server_Characteristic_Read_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   IPS_Characteristic_Type_t  Type;
} IPS_Server_Characteristic_Read_Data_t;

#define IPS_SERVER_CHARACTERISTIC_READ_DATA_SIZE         (sizeof(IPS_Server_Characteristic_Read_Data_t))

   /* The following is dispatched to a IPS Server to inform the         */
   /* application that an IPS Client has written an IPS Characteristic. */
   /* This event indicates that the IPS Server should refresh the       */
   /* broadcast information since at least one IPS Characteristic has   */
   /* been updated.                                                     */
   /* * NOTE * The Characteristic field is a bit mask of the form       */
   /*          IPS_CHARACTERISTIC_XXX, where XXX indicates the IPS      */
   /*          Characteristic that has been updated.  Since multiple IPS*/
   /*          Characteristics may be have been updated (reliable       */
   /*          write), the Characteristic field allows the IPS Server to*/
   /*          determine the IPS Characteristics that have been updated.*/
typedef struct _tagIPS_Server_Characteristic_Updated_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   unsigned int            TransactionID;
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDevice;
   Word_t                  Characteristic;
} IPS_Server_Characteristic_Updated_Data_t;

#define IPS_SERVER_CHARACTERISTIC_UPDATED_DATA_SIZE      (sizeof(IPS_Server_Characteristic_Updated_Data_t))

   /* The following is dispatched to a IPS Server to inform the         */
   /* application that an IPS Client has requested to write an IPS      */
   /* Characteristic or prepare an IPS Characteristic to be written     */
   /* (reliable write or write long).  This means that the IPS Server   */
   /* MUST first authorize the request before the IPS Characteristic(s) */
   /* can be written or prepared.                                       */
   /* ** NOTE ** If this event is received then the                     */
   /*            IPS_Authorization_Request_Response() function MUST be  */
   /*            called in order for the IPS Server to successfully     */
   /*            respond to the request.  Otherwise the outstanding     */
   /*            request issued by the IPS Client will timeout.         */
   /* * NOTE * The IPS Characteristic type that the authorization       */
   /*          request is for will be identified by the Type field.     */
   /* * NOTE * If the IPS Client that made the request is authorized via*/
   /*          the IPS_Authorization_Request_Response(), then the       */
   /*          etIPS_Server_Characteristic_Updated event will be        */
   /*          dispatched when the IPS Characteristic(s) have been      */
   /*          written.                                                 */
typedef struct _tagIPS_Server_Authorization_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   IPS_Characteristic_Type_t  Type;
} IPS_Server_Authorization_Request_Data_t;

#define IPS_SERVER_AUTHORIZATION_REQUEST_DATA_SIZE       (sizeof(IPS_Server_Authorization_Request_Data_t))

   /* The following structure holds all IPS Service Event Data. This    */
   /* structure is received for each event generated. The               */
   /* Event_Data_Type member is used to determine the appropriate union */
   /* member element to access the contained data. The Event_Data_Size  */
   /* member contains the total size of the data contained in this      */
   /* event.                                                            */
typedef struct _tagIPS_Event_Data_t
{
   IPS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      IPS_Server_Characteristic_Read_Data_t    *IPS_Server_Characteristic_Read_Data;
      IPS_Server_Characteristic_Updated_Data_t *IPS_Server_Characteristic_Updated_Data;
      IPS_Server_Authorization_Request_Data_t  *IPS_Server_Authorization_Request_Data;
   } Event_Data;
} IPS_Event_Data_t;

#define IPS_EVENT_DATA_SIZE                              (sizeof(IPS_Event_Data_t))

   /* IPS Client API Structures, Enums, and Constants.                  */

   /* The following structure contains the handles that will need to be */
   /* cached by a IPS client in order to only do service discovery      */
   /* once.                                                             */
typedef struct _tagIPS_Client_Information_t
{
   Word_t  IPS_Indoor_Positioning_Configuration;
   Word_t  IPS_Indoor_Positioning_Configuration_Extended_Properties;
   Word_t  IPS_Latitude;
   Word_t  IPS_Latitude_Extended_Properties;
   Word_t  IPS_Longitude;
   Word_t  IPS_Longitude_Extended_Properties;
   Word_t  IPS_Local_North_Coordinate;
   Word_t  IPS_Local_North_Coordinate_Extended_Properties;
   Word_t  IPS_Local_East_Coordinate;
   Word_t  IPS_Local_East_Coordinate_Extended_Properties;
   Word_t  IPS_Floor_Number;
   Word_t  IPS_Floor_Number_Extended_Properties;
   Word_t  IPS_Altitude;
   Word_t  IPS_Altitude_Extended_Properties;
   Word_t  IPS_Uncertainty;
   Word_t  IPS_Uncertainty_Extended_Properties;
   Word_t  IPS_Location_Name;
   Word_t  IPS_Location_Name_Extended_Properties;
} IPS_Client_Information_t;

#define IPS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(IPS_Client_Information_t))

   /* The following structure will store the decoded broadcast data.    */
   /* * NOTE * Decoded does NOT mean that the encoded values have been  */
   /*          decoded, simply that multi-octet values have been        */
   /*          converted from Little-Endian.                            */
   /* * NOTE * The Flags field is a bit mask of values having hte form  */
   /*          IPS_INDOOR_POSITIONING_CONFIG_XXX, in IPSTypes.h, where  */
   /*          XXX indicates the bit value.                             */
typedef struct _tagIPS_Broadcast_Data_t
{
   Byte_t    Flags;
   SDWord_t  Latitude;
   SDWord_t  Longitude;
   SWord_t   LocalNorthCoordinate;
   SWord_t   LocalEastCoordinate;
   Byte_t    TxPower;
   Byte_t    FloorNumber;
   SWord_t   Altitude;
   Byte_t    Uncertainty;
} IPS_Broadcast_Data_t;

#define IPS_BROADCAST_DATA_SIZE                          (sizeof(IPS_Broadcast_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a IPS Service Event Callback. This function will be called        */
   /* whenever a IPS Service Event occurs that is associated with the   */
   /* specified Bluetooth Stack ID. This function passes to the caller  */
   /* the Bluetooth Stack ID, the IPS Event Data that occurred, and the */
   /* IPS Service Event Callback Parameter that was specified when this */
   /* Callback was installed. The caller is free to use the contents of */
   /* the IPS Service Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data        */
   /* Buffer. This function is guaranteed NOT to be invoked more than   */
   /* once simultaneously for the specified installed callback (i.e.    */
   /* this function DOES NOT have be re-entrant). It needs to be noted  */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially. Because of this, the  */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the       */
   /* Thread Context of a Thread that the User does NOT own. Therefore, */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another IPS Service Event     */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT block and wait for events that    */
   /*          can only be satisfied by receiving IPS Service Event     */
   /*          Packets. A Deadlock WILL occur because NO IPS Event      */
   /*          Callbacks will be issued while this function is          */
   /*          currently outstanding.                                   */
typedef void (BTPSAPI *IPS_Event_Callback_t)(unsigned int BluetoothStackID, IPS_Event_Data_t *IPS_Event_Data, unsigned long CallbackParameter);

   /* IPS Server API.                                                   */

   /* The following function is responsible for opening a IPS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the IPS Service Flags  */
   /* (IPS_SERVICE_FLAGS_...) from IPSTypes.h.  These flags MUST be used*/
   /* to register the GATT service for LE, BR/EDR, or both.  The third  */
   /* parameter is an pointer to information that is used to enable     */
   /* optional service features (characteristics and optional           */
   /* properties), and initialize characteristic values.  The fourth    */
   /* parameter is the Callback function to call when an event occurs on*/
   /* this Server Port.  The fifth parameter is a user-defined callback */
   /* parameter that will be passed to the callback function with each  */
   /* event.  The final parameter is a pointer to store the GATT Service*/
   /* ID of the registered IPS service.  This function returns the      */
   /* positive, non-zero, Instance ID or a negative error code.         */
   /* * NOTE * Only 1 IPS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatched to the            */
   /*          EventCallback function that is specified by the second   */
   /*          parameter to this function.                              */
   /* * NOTE * If BR/EDR is not supported by the Bluetopia configuration*/
   /*          then the IPS_SERVICE_FLAGS_BR_EDR bit will be cleared in */
   /*          the Flags parameter and the SDP Record will not be       */
   /*          registered.  If the IPS_SERVICE_FLAGS_LE bit is also set,*/
   /*          then this function will still return success even though */
   /*          the GATT service was not registered for BR/EDR.          */
BTPSAPI_DECLARATION int BTPSAPI IPS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, IPS_Initialize_Data_t *InitializeInfo, IPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, IPS_Initialize_Data_t *InitializeInfo, IPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a IPS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the IPS Service Flags  */
   /* (IPS_SERVICE_FLAGS_...) from IPSTypes.h.  These flags MUST be used*/
   /* to register the GATT service for LE, BR/EDR, or both.  The third  */
   /* parameter is an pointer to information that is used to enable     */
   /* optional service features (characteristics and optional           */
   /* properties), and initialize characteristic values.  The fourth    */
   /* parameter is the Callback function to call when an event occurs on*/
   /* this Server Port.  The fifth parameter is a user-defined callback */
   /* parameter that will be passed to the callback function with each  */
   /* event.  The sixth parameter is a pointer to store the GATT Service*/
   /* ID of the registered IPS service.  The final parameter is a       */
   /* pointer, that on input can be used to control the location of the */
   /* service in the GATT database, and on ouput to store the service   */
   /* handle range.  This function returns the positive, non-zero,      */
   /* Instance ID or a negative error code.                             */
   /* * NOTE * Only 1 IPS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatched to the            */
   /*          EventCallback function that is specified by the second   */
   /*          parameter to this function.                              */
   /* * NOTE * If BR/EDR is not supported by the Bluetopia configuration*/
   /*          then the IPS_SERVICE_FLAGS_BR_EDR bit will be cleared in */
   /*          the Flags parameter and the SDP Record will not be       */
   /*          registered.  If the IPS_SERVICE_FLAGS_LE bit is also set,*/
   /*          then this function will still return success even though */
   /*          the GATT service was not registered for BR/EDR.          */
BTPSAPI_DECLARATION int BTPSAPI IPS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, IPS_Initialize_Data_t *InitializeInfo, IPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, IPS_Initialize_Data_t *InitializeInfo, IPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened IPS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfull call to           */
   /* IPS_Initialize_XXX() API's.  This function returns zero if        */
   /* successful or a negative error code.                              */
BTPSAPI_DECLARATION int BTPSAPI IPS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is used to perform a suspend of the        */
   /* Bluetooth stack.  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that Bluetopia is to use to collapse it's state information into. */
   /* This function can be called with BufferSize and Buffer set to 0   */
   /* and NULL, respectively.  In this case this function will return   */
   /* the number of bytes that must be passed to this function in order */
   /* to successfully perform a suspend (or 0 if an error occurred, or  */
   /* this functionality is not supported).  If the BufferSize and      */
   /* Buffer parameters are NOT 0 and NULL, this function will attempt  */
   /* to perform a suspend of the stack.  In this case, this function   */
   /* will return the amount of memory that was used from the provided  */
   /* buffers for the suspend (or zero otherwise).                      */
BTPSAPI_DECLARATION unsigned long BTPSAPI IPS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_IPS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* IPS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to IPS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI IPS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the IPS Service that is          */
   /* registered with a call to IPS_Initialize_XXX() API's.  The first  */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to IPS_Initialize_XXX() API's.  This function returns the non-zero*/
   /* number of attributes that are contained in a IPS Server or zero on*/
   /* failure.                                                          */
BTPSAPI_DECLARATION unsigned int BTPSAPI IPS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_IPS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for setting the IPS Indoor  */
   /* Positioning Configuration on the specified IPS Instance.  The     */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to IPS_Initialize_XXX() API's.  The final parameter is the   */
   /* IPS Indoor Positioning Configuration to set for the specified IPS */
   /* Instance.  This function returns zero if successful or a negative */
   /* error code.                                                       */
   /* * NOTE * The Configuration parameter is a bitmask made up of bits */
   /*          of the form IPS_CONFIGURATION_FLAG_XXX from IPSTypes.h.  */
   /*          The Configuration may be zero.                           */
   /* * NOTE * If any bits for optional service features that are not   */
   /*          included in the service or reserved for future use bits  */
   /*          are set this function will return                        */
   /*          IPS_ERROR_INVALID_CONFIGURATION.                         */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Indoor_Positioning_Configuration(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Indoor_Positioning_Configuration_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t Configuration);
#endif

   /* The following function is responsible for querying the IPS Indoor */
   /* Positioning Configuration on the specified IPS Instance.  The     */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to IPS_Initialize_XXX() API's.  The final parameter is a     */
   /* pointer to return the IPS Indoor Positioning Configuration for the*/
   /* specified IPS Instance if this function is successful.  This      */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * The Configuration parameter is a bitmask made up of bits */
   /*          of the form IPS_CONFIGURATION_FLAG_XXX from IPSTypes.h.  */
   /*          The Configuration may be zero.                           */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Indoor_Positioning_Configuration(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Indoor_Positioning_Configuration_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *Configuration);
#endif

   /* The following function is responsible for setting the IPS Latitude*/
   /* on the specified IPS Instance.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is the IPS       */
   /* Latitude to set for the specified IPS Instance.  This function    */
   /* returns zero if successful or a negative error code.              */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Latitude(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t Latitude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Latitude_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t Latitude);
#endif

   /* The following function is responsible for setting the IPS Latitude*/
   /* on the specified IPS Instance.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is a pointer to  */
   /* the IPS Latitude that will contain the IPS Latitude for the       */
   /* specified IPS Instance if this function is successful.  This      */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * This function will simply retrieve the encoded Latitude. */
   /*          It is the application's responsiblity to decode the      */
   /*          value.                                                   */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Latitude(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t *Latitude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Latitude_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t *Latitude);
#endif

   /* The following function is responsible for setting the IPS         */
   /* Longitude on the specified IPS Instance.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* IPS_Initialize_XXX() API's.  The final parameter is the IPS       */
   /* Longitude to set for the specified IPS Instance.  This function   */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * This function will simply store the encoded Latitude.  It*/
   /*          is the application's responsiblity to make sure the      */
   /*          encoded value is valid.                                  */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Longitude(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t Longitude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Longitude_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t Longitude);
#endif

   /* The following function is responsible for setting the IPS         */
   /* Longitude on the specified IPS Instance.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* IPS_Initialize_XXX() API's.  The final parameter is a pointer to  */
   /* the IPS Longitude that will contain the IPS Longitude for the     */
   /* specified IPS Instance if this function is successful.  This      */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * This function will simply retrieve the encoded Longitude.*/
   /*          It is the application's responsiblity to decode the      */
   /*          value.                                                   */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Longitude(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t *Longitude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Longitude_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SDWord_t *Longitude);
#endif

   /* The following function is responsible for setting the IPS Local   */
   /* North Coordinate on the specified IPS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to IPS_Initialize_XXX() API's.  The final parameter is the IPS    */
   /* Local North Coordinate to set for the specified IPS Instance.     */
   /* This function returns zero if successful or a negative error code.*/
   /* * NOTE * If the IPS Local North Coordinate is not supported by the*/
   /*          service then the error code                              */
   /*          IPS_ERROR_FEATURE_NOT_SUPPORTED will be returned.        */
   /* * NOTE * This function will simply store the encoded Local North  */
   /*          Coordinate.  It is the application's responsiblity to    */
   /*          make sure the encoded value is valid.                    */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Local_North_Coordinate(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t LocalCoordinate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Local_North_Coordinate_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t LocalCoordinate);
#endif

   /* The following function is responsible for setting the IPS Local   */
   /* North Coordinate on the specified IPS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to IPS_Initialize_XXX() API's.  The final parameter is a pointer  */
   /* to the IPS Local North Coordinate that will contain the IPS Local */
   /* North Coordinate for the specified IPS Instance if this function  */
   /* is successful.  This function returns zero if successful or a     */
   /* negative error code.                                              */
   /* * NOTE * If the IPS Local North Coordinate is not supported by the*/
   /*          service then the error code                              */
   /*          IPS_ERROR_FEATURE_NOT_SUPPORTED will be returned.        */
   /* * NOTE * This function will simply retrieve the encoded Local     */
   /*          North Coordinate.  It is the application's responsiblity */
   /*          to decode the value.                                     */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Local_North_Coordinate(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t *LocalCoordinate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Local_North_Coordinate_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t *LocalCoordinate);
#endif

   /* The following function is responsible for setting the IPS Local   */
   /* East Coordinate on the specified IPS Instance.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to IPS_Initialize_XXX() API's.  The final parameter is the IPS    */
   /* Local East Coordinate to set for the specified IPS Instance.  This*/
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * If the IPS Local East Coordinate is not supported by the */
   /*          service then the error code                              */
   /*          IPS_ERROR_FEATURE_NOT_SUPPORTED will be returned.        */
   /* * NOTE * This function will simply store the encoded Local East   */
   /*          Coordinate.  It is the application's responsiblity to    */
   /*          make sure the encoded value is valid.                    */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Local_East_Coordinate(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t LocalCoordinate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Local_East_Coordinate_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t LocalCoordinate);
#endif

   /* The following function is responsible for setting the IPS Local   */
   /* East Coordinate on the specified IPS Instance.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to IPS_Initialize_XXX() API's.  The final parameter is a pointer  */
   /* to the IPS Local East Coordinate that will contain the IPS Local  */
   /* East Coordinate for the specified IPS Instance if this function is*/
   /* successful.  This function returns zero if successful or a        */
   /* negative error code.                                              */
   /* * NOTE * If the IPS Local East Coordinate is not supported by the */
   /*          service then the error code                              */
   /*          IPS_ERROR_FEATURE_NOT_SUPPORTED will be returned.        */
   /* * NOTE * This function will simply retrieve the encoded Local East*/
   /*          Coordinate.  It is the application's responsiblity to    */
   /*          decode the value.                                        */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Local_East_Coordinate(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t *LocalCoordinate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Local_East_Coordinate_t)(unsigned int BluetoothStackID, unsigned int InstanceID, SWord_t *LocalCoordinate);
#endif

   /* The following function is responsible for setting the IPS Floor   */
   /* Number on the specified IPS Instance.  The first parameter is the */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is the IPS Floor */
   /* Number to set for the specified IPS Instance.  This function      */
   /* returns zero if successful or a negative error code.              */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Floor_Number(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t FloorNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Floor_Number_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t FloorNumber);
#endif

   /* The following function is responsible for setting the IPS Floor   */
   /* Number on the specified IPS Instance.  The first parameter is the */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is a pointer to  */
   /* the IPS Floor Number that will contain the IPS Floor Number for   */
   /* the specified IPS Instance if this function is successful.  This  */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * If the IPS Floor Number is not supported by the service  */
   /*          then the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will */
   /*          be returned.                                             */
   /* * NOTE * This function will simply retrieve the encoded Floor     */
   /*          Number.  It is the application's responsiblity to decode */
   /*          the value.                                               */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Floor_Number(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *FloorNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Floor_Number_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *FloorNumber);
#endif

   /* The following function is responsible for setting the IPS Altitude*/
   /* on the specified IPS Instance.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is the IPS       */
   /* Altitude to set for the specified IPS Instance.  This function    */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * If the IPS Altitude is not supported by the service then */
   /*          the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will be   */
   /*          returned.                                                */
   /* * NOTE * This function will simply store the encoded Altitude.  It*/
   /*          is the application's responsiblity to make sure the      */
   /*          encoded value is valid.                                  */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Altitude(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t Altitude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Altitude_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t Altitude);
#endif

   /* The following function is responsible for setting the IPS Altitude*/
   /* on the specified IPS Instance.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is a pointer to  */
   /* the IPS Altitude that will contain the IPS Altitude for the       */
   /* specified IPS Instance if this function is successful.  This      */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * If the IPS Altitude is not supported by the service then */
   /*          the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will be   */
   /*          returned.                                                */
   /* * NOTE * This function will simply retrieve the encoded Altitude. */
   /*          It is the application's responsiblity to decode the      */
   /*          value.                                                   */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Altitude(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *Altitude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Altitude_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *Altitude);
#endif

   /* The following function is responsible for setting the IPS         */
   /* Uncertainty on the specified IPS Instance.  The first parameter is*/
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* IPS_Initialize_XXX() API's.  The final parameter is the IPS       */
   /* Uncertainty to set for the specified IPS Instance.  This function */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * If the IPS Uncertainty is not supported by the service   */
   /*          then the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will */
   /*          be returned.                                             */
   /* * NOTE * This function will simply store the Uncertainty bit mask.*/
   /*          It is the application's responsiblity to make sure the   */
   /*          encoded value is valid.                                  */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Uncertainty(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t Uncertainty);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Uncertainty_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t Uncertainty);
#endif

   /* The following function is responsible for setting the IPS         */
   /* Uncertainty on the specified IPS Instance.  The first parameter is*/
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* IPS_Initialize_XXX() API's.  The final parameter is a pointer to  */
   /* the IPS Uncertainty that will contain the IPS Uncertainty for the */
   /* specified IPS Instance if this function is successful.  This      */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * If the IPS Uncertainty is not supported by the service   */
   /*          then the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will */
   /*          be returned.                                             */
   /* * NOTE * This function will simply retrieve the encoded           */
   /*          Uncertainty.  It is the application's responsiblity to   */
   /*          decode the value.                                        */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Uncertainty(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *Uncertainty);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Uncertainty_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *Uncertainty);
#endif

   /* The following function is responsible for setting the IPS Location*/
   /* Name on the specified IPS Instance.  The first parameter is the   */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The final parameter is the IPS       */
   /* Location Name to set for the specified IPS Instance.  This        */
   /* function returns zero if successful or a negative error code.     */
   /* * NOTE * If the IPS Location Name is not supported by the service */
   /*          then the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will */
   /*          be returned.                                             */
   /* * NOTE * The LocationName parameter MUST be less than or equal to */
   /*          BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH in*/
   /*          size including the NULL terminator.  Otherwise the error */
   /*          code IPS_ERROR_INVALID_STRING_LENGTH will be returned.   */
BTPSAPI_DECLARATION int BTPSAPI IPS_Set_Location_Name(unsigned int BluetoothStackID, unsigned int InstanceID, char *LocationName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Set_Location_Name_t)(unsigned int BluetoothStackID, unsigned int InstanceID, char *LocationName);
#endif

   /* The following function is responsible for setting the IPS Location*/
   /* Name on the specified IPS Instance.  The first parameter is the   */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* IPS_Initialize_XXX() API's.  The third parameter is the size of   */
   /* the buffer that will hold the Location Name parameter.  This      */
   /* parameter MUST be at least the minimum size of the LocationName   */
   /* with room to store the NULL terminator.  The final parameter is a */
   /* pointer to the IPS Location Name that will contain the IPS        */
   /* Location Name for the specified IPS Instance if this function is  */
   /* successful.  This value MUST be big enough to hold the IPS        */
   /* Location Name.  This function returns zero if successful or a     */
   /* negative error code.                                              */
   /* * NOTE * If the IPS Location Name is not supported by the service */
   /*          then the error code IPS_ERROR_FEATURE_NOT_SUPPORTED will */
   /*          be returned.                                             */
   /* * NOTE * The LocationName parameter MUST be large enough to hold  */
   /*          the IPS Location Name and the NULL terminator that this  */
   /*          function will automatically set.  The IPS Location Name  */
   /*          cannot be greater than                                   */
   /*          BTPS_CONFIGURATION_IPS_MAXIMUM_SUPPORTED_STRING_LENGTH in*/
   /*          size.  Otherwise the error code                          */
   /*          IPS_ERROR_INVALID_STRING_LENGTH will be returned.        */
   /* * NOTE * If LocationName has not been previuosly set then the     */
   /*          error code IPS_ERROR_LOCATION_NAME_NOT_CONFIGURED will be*/
   /*          returned.  Use IPS_Set_Location_Name() to configure the  */
   /*          Location Name.                                           */
BTPSAPI_DECLARATION int BTPSAPI IPS_Query_Location_Name(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int LocationNameLength, char *LocationName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Query_Location)Name_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int LocationNameLength, char *LocationName);
#endif

   /* The following function is responsible for responding to an        */
   /* authorization request that has been dispatched by the IPS Server  */
   /* for the specified IPS instance.  An authorization request event   */
   /* will be dispatched if the IPS Server has been configured to       */
   /* require authorization before writing or preparing (reliable write */
   /* and write long) IPS Characteristics, and an IPS Characteristic has*/
   /* been requested to be written or prepared.  The first parameter is */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* IPS_Initialize_XXX() API's.  The third parameter is the GATT      */
   /* Transaction ID that triggered the event.  The final parameter is a*/
   /* boolean to reject the request if TRUE or accept the request if    */
   /* FALSE, from the IPS Client.                                       */
   /* * NOTE * The IPS Server can be configured to require authorization*/
   /*          for write requests by enabling the                       */
   /*          Enable_Authorized_Device_Write field of the              */
   /*          IPS_Initialize_Data_t structure when the IPS Server is   */
   /*          initialized by a call to the IPS_Initialize_XXX()        */
   /*          functions.                                               */
BTPSAPI_DECLARATION int BTPSAPI IPS_Authorization_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Boolean_t NotAuthorized);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Authorization_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Boolean_t NotAuthorized);
#endif

   /* The following function is a utility function for determining the  */
   /* size of the IPS Broadcast Data or formatting the IPS Broadcast    */
   /* Data based on the IPS Indoor Positioning Configuration for the    */
   /* specified IPS Instance.  The first parameter is the Bluetooth     */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to IPS_Initialize_XXX()*/
   /* API's.  The third parameter, on input, is the length of the IPS   */
   /* Broadcast Data Buffer and will contain the size of the formatted  */
   /* IPS Broadcast Data Buffer on output.  The final parameter if not  */
   /* NULL, is the IPS Broadcast Data Buffer, which will contain on     */
   /* output, the formatted IPS Broadcast Data, if this function is     */
   /* successful.  This function returns zero if successful or a        */
   /* negative error code.                                              */
   /* * NOTE * The BufferLength parameter is MANDATORY (cannot be NULL).*/
   /*          This parameter cannot be greater than                    */
   /*          IPS_MAXIMUM_BROADCAST_DATA_SIZE.                         */
   /* * NOTE * If either the BufferLength parameter is zero or the      */
   /*          Buffer is NULL, this function will simply return the     */
   /*          expected size of the IPS Broadcast Data Buffer.          */
BTPSAPI_DECLARATION int BTPSAPI IPS_Format_Broadcasting_Data(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Format_Broadcasting_Data_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *BufferLength, Byte_t *Buffer);
#endif

   /* IPS Client API.                                                   */

   /* The following function is a utility function that is responsible  */
   /* for decoding the broadcasting data.  The first parameter is a     */
   /* pointer to the Buffer that contains the Broadcast information     */
   /* received during scanning.  The second parameter is a the length of*/
   /* the Buffer.  The final parameter is a pointer to a structure that */
   /* will contain the decoded Broadcast information.  This function    */
   /* returns zero if successful, or a negative error code.             */
   /* * NOTE * This function Expects the AD_Data_Buffer and             */
   /*          AD_Data_Length fields as the Buffer and BufferLength     */
   /*          parameters from the GAP_LE_Advertising_Data_Entry_t      */
   /*          structure.  This structure stores the mandatory Length   */
   /*          and AD Type fields separately from the rest of the       */
   /*          broadcast information that is in the Buffer.             */
   /* * NOTE * This function should not be called if the AD_Data_Length */
   /*          field is 1.  This indicates that only the AD Type: Indoor*/
   /*          Positioning Service has been included.  The Flags field  */
   /*          has also been omitted since no other data has been       */
   /*          included.                                                */
BTPSAPI_DECLARATION int BTPSAPI IPS_Decode_Broadcasting_Data(Byte_t BufferLength, Byte_t *Buffer, IPS_Broadcast_Data_t *BroadcastData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IPS_Decode_Broadcasting_Data_t)(Byte_t BufferLength, Byte_t *Buffer, IPS_Broadcast_Data_t *BroadcastData);
#endif

#endif
