/*****< blsapi.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLSAPI - Stonestreet One Bluetooth Blood Pressure Service                 */
/*           (GATT based) API Type Definitions, Constants, and Prototypes.    */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/11/12  A. Parashar    Initial creation.                               */
/******************************************************************************/
#ifndef __BLSAPIH__
#define __BLSAPIH__

#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.      */
#include "SS1BTGAT.h"       /* Bluetooth Stack GATT API Prototypes/Constants. */
#include "BLSTypes.h"       /* Blood Pressure Service Types/Constants.        */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BLS_ERROR_INVALID_PARAMETER                       (-1000)
#define BLS_ERROR_INVALID_BLUETOOTH_STACK_ID              (-1001)
#define BLS_ERROR_INSUFFICIENT_RESOURCES                  (-1002)
#define BLS_ERROR_INSUFFICIENT_BUFFER_SPACE               (-1003)
#define BLS_ERROR_SERVICE_ALREADY_REGISTERED              (-1004)
#define BLS_ERROR_INVALID_INSTANCE_ID                     (-1005)
#define BLS_ERROR_MALFORMATTED_DATA                       (-1006)
#define BLS_ERROR_INDICATION_OUTSTANDING                  (-1007)

   /* The following structure contains the Handles that will need to be */
   /* cached by a BLS client in order to only do service discovery once */
typedef struct _tagBLS_Client_Information_t
{
   Word_t Blood_Pressure_Measurement;
   Word_t Blood_Pressure_Measurement_Client_Configuration;
   Word_t Intermediate_Cuff_Pressure;
   Word_t Intermediate_Cuff_Pressure_Client_Configuration;
   Word_t Blood_Pressure_Feature;
} BLS_Client_Information_t;

#define BLS_CLIENT_INFORMATION_DATA_SIZE                  (sizeof(BLS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a BLS Server.                           */
typedef struct _tagBLS_Server_Information_t
{
   Word_t Blood_Pressure_Measurement_Client_Configuration;
   Word_t Intermediate_Cuff_Pressure_Client_Configuration;
} BLS_Server_Information_t;

#define BLS_SERVER_INFORMATION_DATA_SIZE                  (sizeof(BLS_Server_Information_t))

   /* The following define the valid Read Request types that a server   */
   /* may receive in a etBLS_Server_Read_Client_Configuration_Request or*/
   /* etBLS_Server_Client_Configuration_Update event. This is also used */
   /* by the BLS_Send_Notification to denote the characteristic value to*/
   /* notify.                                                           */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctBloodPressureMeasurement,
   ctIntermediateCuffPressure
} BLS_Characteristic_Type_t;

   /* The following structure defines the structure of the Blood        */
   /* Pressure Measurement Time stamp field parameter that may be       */
   /* included in Blood Pressure Measurement.                           */
typedef struct _tagBLS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} BLS_Date_Time_Data_t;

#define BLS_DATE_TIME_DATA_SIZE                           (sizeof(BLS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified Time Stamp is valid.  The only parameter to this        */
   /* function is the BLS_Date_Time_Data_t structure to valid.This MACRO*/
   /* returns TRUE if the Time Stamp is valid or FALSE otherwise.       */
#define BLS_TIME_STAMP_VALID(_x)              ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following defines the structure of the Blood Pressure         */
   /* Measurement Compound Value included in the Blood Pressure         */
   /* Measurement characteristic.                                       */
   /* * NOTE * If a value for Systolic, Diastolic or MAP subfields is   */
   /*          unavailable , the special short float value NaN will be  */
   /*          filled in each of the unavailable subfields.             */
typedef struct _tagBLS_Compound_Value_Data_t
{
   Word_t Systolic;
   Word_t Diastolic;
   Word_t Mean_Arterial_Pressure;
}BLS_Compound_Value_Data_t;

#define BLS_COMPOUND_VALUE_DATA_SIZE                       (sizeof(BLS_Compound_Value_Data_t))

   /* The following defines the structure of the Blood Pressure         */
   /* Measurement Data that is passed to the function that builds the   */
   /* Blood Pressure Measurement packet.                                */
typedef struct _tagBLS_Blood_Pressure_Measurement_Data_t
{
   Byte_t                    Flags;
   BLS_Compound_Value_Data_t CompoundValue;
   BLS_Date_Time_Data_t      TimeStamp;
   Word_t                    PulseRate;
   Byte_t                    UserID;
   Word_t                    MeasurementStatus;
   Byte_t                    Reserved[2];
} BLS_Blood_Pressure_Measurement_Data_t;

#define BLS_BLOOD_PRESSURE_MEASUREMENT_DATA_SIZE          (sizeof(BLS_Blood_Pressure_Measurement_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* BLS Profile.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the BLS_Event_Data_t structure.                                   */
typedef enum
{
   etBLS_Read_Client_Configuration_Request,
   etBLS_Client_Configuration_Update,
   etBLS_Confirmation_Data
} BLS_Event_Type_t;

   /* The following is dispatched to a BLS Server when a BLS Client     */
   /* is attempting to read the Client Configuration descriptor.  The   */
   /* ConnectionID, and RemoteDevice identifies the Client that is      */
   /* making the request.  The TransactionID specifies the TransactionID*/
   /* of the request, this can be used when responding to the request   */
   /* using the BLS_Client_Configuration_Read_Response() API function.  */
typedef struct _tagBLS_Read_Client_Configuration_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   BLS_Characteristic_Type_t ClientConfigurationType;
} BLS_Read_Client_Configuration_Data_t;

#define BLS_READ_CLIENT_CONFIGURATION_DATA_SIZE           (sizeof(BLS_Read_Client_Configuration_Data_t))

   /* The following is dispatched to a BLS Server when a BLS Client     */
   /* attempts to write to a Client Configuration descriptor.  The      */
   /* ConnectionID and RemoteDevice identify the Client that is making  */
   /* the update request.The ClientConfiguration value specifies the    */
   /* new Client Configuration value.                                   */
typedef struct _tagBLS_Client_Configuration_Update_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   BLS_Characteristic_Type_t ClientConfigurationType;
   Word_t                    ClientConfiguration;
} BLS_Client_Configuration_Update_Data_t;

#define BLS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE         (sizeof(BLS_Client_Configuration_Update_Data_t))

   /* The following BLS Profile Event is dispatched to a BLS Server     */
   /* when a BLS Client has sent a confirmation to a previously sent    */
   /* confirmation. The ConnectionID, ConnectionType, and RemoteDevice  */
   /* specifiy the Client that is making the update.  The               */
   /* Characteristic_Type specifies which Indication the Client has sent*/
   /* a confirmation for.  The final parameter specifies the status of  */
   /* the Indication                                                    */
   /* * NOTE * The Status member is set to one of the following values: */
   /*                GATT_CONFIRMATION_STATUS_SUCCESS                   */
   /*                GATT_CONFIRMATION_STATUS_TIMEOUT                   */
typedef struct _tagBLS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   Byte_t                 Status;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} BLS_Confirmation_Data_t;

#define BLS_CONFIRMATION_DATA_SIZE                        (sizeof(BLS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all BLS Profile Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.The Event_Data_Size member contains the total      */
   /* size of the data contained in this event.                         */
typedef struct _tagBLS_Event_Data_t
{
   BLS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      BLS_Read_Client_Configuration_Data_t   *BLS_Read_Client_Configuration_Data;
      BLS_Client_Configuration_Update_Data_t *BLS_Client_Configuration_Update_Data;
      BLS_Confirmation_Data_t                *BLS_Confirmation_Data;
   } Event_Data;
}BLS_Event_Data_t;

#define BLS_EVENT_DATA_SIZE                               (sizeof(BLS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a BLS Profile Event Receive Data Callback.  This function will    */
   /* be called whenever an BLS Profile Event occurs that is            */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the BLS Event Data   */
   /* that occurred and the BLS Profile Event Callback Parameter that   */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the BLS Profile Event Data ONLY in    */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* re-entrant).  It needs to be noted however, that if the same      */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another BLS Profile Event will not be processed while this        */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving BLS Profile Event   */
   /*            Packets.  A Deadlock WILL occur because NO BLS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *BLS_Event_Callback_t)(unsigned int BluetoothStackID, BLS_Event_Data_t *BLS_Event_Data, unsigned long CallbackParameter);

   /* BLS Server API.                                                   */

   /* The following function is responsible for opening a BLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered BLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 BLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI BLS_Initialize_Service(unsigned int BluetoothStackID, BLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Initialize_Service_t)(unsigned int BluetoothStackID, BLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a BLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered BLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 BLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI BLS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, BLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, BLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened BLS Server.  The first parameter is the Bluetooth Stack    */
   /* ID on which to close the server.  The second parameter is the     */
   /* InstanceID that was returned from a successful call to            */
   /* BLS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI BLS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI BLS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_BLS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* BLS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to BLS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI BLS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the BLS Service that is          */
   /* registered with a call to BLS_Initialize_Service().  This function*/
   /* returns the non-zero number of attributes that are contained in a */
   /* BLS Server or zero on failure.                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI BLS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_BLS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a BLS Read*/
   /* Client Configuration Request.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* BLS_Initialize_Server().The third is the Transaction ID of the    */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI BLS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for sending a Measurement   */
   /* Data indication to a specified remote device.The first parameter  */
   /* is the Bluetooth Stack ID of the Bluetooth Device. The second     */
   /* parameter is the InstanceID returned from a successful call       */
   /* to BLS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the indication to. The  */
   /* final parameter is the Measurement data to indicate.This function */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BLS_Indicate_Blood_Pressure_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, BLS_Blood_Pressure_Measurement_Data_t *Measurement_Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Indicate_Temperature_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, BLS_Blood_Pressure_Measurement_Data_t *Measurement_Data);
#endif

   /* The following function is responsible for sending an Intermediate */
   /* Cuff Pressure Data notification to a specified remote device.     */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device. The second parameter is the InstanceID returned from a    */
   /* successful call to BLS_Initialize_Server().  The third parameter  */
   /* is the ConnectionID of the remote device to send the notification */
   /* to. The final parameter is the Intermediate Cuff Pressure Data    */
   /* to notify. This function returns a zero if successful or a        */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI BLS_Notify_Intermediate_Cuff_Pressure(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, BLS_Blood_Pressure_Measurement_Data_t *Intermediate_Cuff_Pressure);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Notify_Intermediate_Cuff_Pressure_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, BLS_Blood_Pressure_Measurement_Data_t *Intermediate_Cuff_Pressure);
#endif

   /* The following function is responsible for Writing the BLS         */
   /* Feature on the specified BLS Instance.The first parameter is      */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is InstanceID returned from a successful call to                  */
   /* BLS_Initialize_Server The final parameter is a pointer to return  */
   /* the BLS Feature for the specified BLS Instance.  This function    */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BLS_Set_Blood_Pressure_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Set_Blood_Pressure_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures);
#endif

   /* The following function is responsible for querying the BLS        */
   /* Feature on the specified BLS Instance.The first parameter is      */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is InstanceID returned from a successful call to                  */
   /* BLS_Initialize_Server The final parameter is a pointer to return  */
   /* the BLS Feature for the specified BLS Instance.  This function    */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BLS_Query_Blood_Pressure_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Query_Blood_Pressure_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures);
#endif

   /* HTS Client API.                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote BLS Server interpreting it as a Blood Pressure      */
   /* Measurement characteristic.  The first parameter is the length of */
   /* the value returned by the remote BLS Server.  The second parameter*/
   /* is a pointer to the data returned by the remote BLS Server.  The  */
   /* final parameter is a pointer to store the parsed Blood Pressure   */
   /* Measurement value.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
BTPSAPI_DECLARATION int BTPSAPI BLS_Decode_Blood_Pressure_Measurement(unsigned int ValueLength, Byte_t *Value, BLS_Blood_Pressure_Measurement_Data_t *BloodPressureMeasurement);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLS_Decode_Blood_Pressure_Measurement_t)(unsigned int ValueLength, Byte_t *Value, BLS_Blood_Pressure_Measurement_Data_t *BloodPressureMeasurement);
#endif

#endif
