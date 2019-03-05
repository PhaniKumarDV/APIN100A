/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< wssapi.h >*************************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  WSSAPI - Qualcomm Technologies Bluetooth Weight Scale Service (GATT       */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/25/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __WSSAPIH__
#define __WSSAPIH__

#include "SS1BTPS.h"         /* Bluetooth Stack API Prototypes/Constants.     */
#include "SS1BTGAT.h"        /* Bluetooth Stack GATT API Prototypes/Constants.*/
#include "WSSTypes.h"        /* Immediate Alert Service Types/Constants.      */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define WSS_ERROR_INVALID_PARAMETER                      (-1000)
#define WSS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define WSS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define WSS_ERROR_INSUFFICIENT_BUFFER_SPACE              (-1003)
#define WSS_ERROR_SERVICE_ALREADY_REGISTERED             (-1004)
#define WSS_ERROR_INVALID_INSTANCE_ID                    (-1005)
#define WSS_ERROR_MALFORMATTED_DATA                      (-1006)

   /* The following structure contains the Handles that will need to be */
   /* cached by a WSS client in order to only do service discovery once.*/
typedef struct _tagWSS_Client_Information_t
{
   Word_t Weight_Scale_Feature;
   Word_t Weight_Measurement;
   Word_t Weight_Measurement_CCCD;
} WSS_Client_Information_t;

#define WSS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(WSS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a WSS Server.                           */
typedef struct _tagWSS_Server_Information_t
{
   Word_t Weight_Measurement_Configuration;
} WSS_Server_Information_t;

#define WSS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(WSS_Server_Information_t))

   /* The following structure represents the format of a WSS Date/Time  */
   /* value.  This is used to represent the Date/Time which contains the*/
   /* Day/Month/Year and Hours:Minutes:Second data.                     */
   /* * NOTE * The Month and Day start at 1 for the first month and     */
   /*          first day.                                               */
typedef struct _tagWSS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
}  WSS_Date_Time_Data_t;

#define WSS_DATE_TIME_DATA_SIZE                          (sizeof(WSS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date Time is valid.  The only parameter to this  */
   /* function is the WSS_Date_Time_Data_t structure to validate.  This*/
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
#define WSS_DATE_TIME_VALID(_x)                          ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following structure represents the WSS Weight Measurement     */
   /* data.                                                             */
   /* * NOTE * The flags field is a bit mask that is used to control the*/
   /*          optional fields that can be included in a Weight         */
   /*          Measurement.  The bit mask values have the following     */
   /*          form: WSS_WEIGHT_MEASUREMENT_FLAG_XXX, and can be found  */
   /*          in WSSTypes.h.                                           */
   /* * NOTE * The value WSS_WEIGHT_MEASUREMENT_UNSUCCESSFUL may be set */
   /*          for the Weight field to indicate that a measurement was  */
   /*          unsuccessful.  If this is the case all optional fields   */
   /*          other than the Time stamp field and the User ID field    */
   /*          SHALL be disabled.                                       */
   /* * NOTE * The User_ID field SHALL be included if the WSS Server    */
   /*          supports multiple users.  The value WSS_USER_ID_UNKNOWN  */
   /*          may be used to indicate that the user ID is not known if */
   /*          the User_ID field is included in the weight measurement. */
   /* * NOTE * The BMI and Height fields are optional and SHALL be      */
   /*          included together.  The WSS Server MUST know the height  */
   /*          of the user since the BMI is calculated by dividing the  */
   /*          Weight (pounds) by the square of the Height (inches), and*/
   /*          multiplying this value by a factor of approximately      */
   /*          (703.07).                                                */
typedef struct _tagWSS_Weight_Measurement_Data_t
{
   Byte_t               Flags;
   Word_t               Weight;
   WSS_Date_Time_Data_t Time_Stamp;
   Byte_t               User_ID;
   Word_t               BMI;
   Word_t               Height;
} WSS_Weight_Measurement_Data_t;

#define WSS_WEIGHT_MEASUREMENT_DATA_SIZE                 (sizeof(WSS_Weight_Measurement_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* WSS Service.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the WSS_Event_Data_t structure.                                   */
typedef enum _tagWSS_Event_Type_t
{
   etWSS_Server_Read_Weight_Measurement_CCCD_Request,
   etWSS_Server_Write_Weight_Measurement_CCCD_Request,
   etWSS_Server_Confirmation_Data
} WSS_Event_Type_t;

   /* The following event data is dispatched to an WSS Server to inform */
   /* the application that a WSS Client has requested to read the Weight*/
   /* Measurement Characteristic's Client Characteristic Configuration  */
   /* descriptor (CCCD).  The InstanceID field is the identifier for the*/
   /* instance of WSS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the WSS Client and */
   /* WSS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the WSS Client that  */
   /* made the request.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the WSS_Read_Weight_Measurement_CCCD_Request_Response()  */
   /*          function to send the response to the outstanding request.*/
typedef struct _tagWSS_Read_Weight_Measurement_CCCD_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
} WSS_Read_Weight_Measurement_CCCD_Request_Data_t;

#define WSS_READ_WEIGHT_MEASUREMENT_CCCD_REQUEST_DATA_SIZE  (sizeof(WSS_Read_Weight_Measurement_CCCD_Request_Data_t))

   /* The following event data is dispatched to an WSS Server to inform */
   /* the application that a WSS Client has requested to write the      */
   /* Weight Measurement Characteristic's Client Characteristic         */
   /* Configuration descriptor (CCCD).  The InstanceID field is the     */
   /* identifier for the instance of WSS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* WSS Client and WSS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the WSS Client that  */
   /* made the request.  The final field is the ClientConfiguration that*/
   /* contains the value that has been written for the CCCD.            */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the WSS_Write_Weight_Measurement_CCCD_Request_Response() */
   /*          function to send the response to the outstanding request.*/
typedef struct _tagWSS_Write_Weight_Measurement_CCCD_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   Word_t                 ClientConfiguration;
} WSS_Write_Weight_Measurement_CCCD_Request_Data_t;

#define WSS_WRITE_WEIGHT_MEASUREMENT_CCCD_REQUEST_DATA_SIZE  (sizeof(WSS_Write_Weight_Measurement_CCCD_Request_Data_t))

   /* The following is dispatched to a WSS Server when a WSS Client     */
   /* confirms an outstanding indication.  The InstanceID field is the  */
   /* identifier for the instance of WSS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* WSS Client and WSS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the WSS Client that  */
   /* made the request.  The Status field indicates the status of the   */
   /* outstanding indication.  The BytesWritten field indicates the     */
   /* number of bytes that were successfully written in the outstanding */
   /* indication.                                                       */
   /* * NOTE * This event does not have a response.                     */
typedef struct _tagWSS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
   Word_t                 BytesWritten;
} WSS_Confirmation_Data_t;

#define WSS_CONFIRMATION_DATA_SIZE                       (sizeof(WSS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all WSS Service Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagWSS_Event_Data_t
{
   WSS_Event_Type_t Event_Data_Type;
   Byte_t           Event_Data_Size;
   union
   {
      WSS_Read_Weight_Measurement_CCCD_Request_Data_t  *WSS_Read_Weight_Measurement_CCCD_Request_Data;
      WSS_Write_Weight_Measurement_CCCD_Request_Data_t *WSS_Write_Weight_Measurement_CCCD_Request_Data;
      WSS_Confirmation_Data_t                          *WSS_Confirmation_Data;
   } Event_Data;
} WSS_Event_Data_t;

#define WSS_EVENT_DATA_SIZE                              (sizeof(WSS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a WSS Service Event Receive Data Callback.  This function will be */
   /* called whenever an WSS Service Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the WSS Event Data that        */
   /* occurred and the WSS Service Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the WSS Service Event Data ONLY in  context   */
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer This function is guaranteed NOT to be invoked */
   /* more than once simultaneously for the specified installed callback*/
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another WSS Service  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving WSS Service Event   */
   /*            Packets.  A Deadlock WILL occur because NO WSS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *WSS_Event_Callback_t)(unsigned int BluetoothStackID, WSS_Event_Data_t *WSS_Event_Data, unsigned long CallbackParameter);

   /* WSS Server API.                                                   */

   /* The following function is responsible for opening a WSS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the WSS Service Flags  */
   /* (WSS_SERVICE_FLAGS_XXX) found in WSSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an HPS  */
   /* Client, for the specified transport.  The third parameter is the  */
   /* Callback function to call when an event occurs on this Server     */
   /* Port.  The fourth parameter is a user-defined callback parameter  */
   /* that will be passed to the callback function with each event.  The*/
   /* final parameter is a pointer to store the GATT Service ID of the  */
   /* registered WSS service.  This can be used to include the service  */
   /* registered by this call.  This function returns the positive,     */
   /* non-zero, Instance ID or a negative error code.                   */
   /* * NOTE * Only 1 WSS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI WSS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, WSS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, WSS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a WSS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the WSS Service Flags  */
   /* (WSS_SERVICE_FLAGS_XXX) found in WSSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an HPS  */
   /* Client, for the specified transport.  The third parameter is the  */
   /* Callback function to call when an event occurs on this Server     */
   /* Port.  The fourth parameter is a user-defined callback parameter  */
   /* that will be passed to the callback function with each event.  The*/
   /* fifth parameter is a pointer to store the GATT Service ID of the  */
   /* registered WSS service.  This can be used to include the service  */
   /* registered by this call.  The final parameter is a pointer, that  */
   /* on input can be used to control the location of the service in the*/
   /* GATT database, and on ouput to store the service handle range.    */
   /* This function returns the positive, non-zero, Instance ID or a    */
   /* negative error code.                                              */
   /* * NOTE * Only 1 WSS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI WSS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, WSS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, WSS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened WSS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfull call to           */
   /* WSS_Initialize_XXX().  This function returns a zero if successful */
   /* or a negative return error code if an error occurs.               */
BTPSAPI_DECLARATION int BTPSAPI WSS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI WSS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_WSS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* WSS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to WSS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI WSS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the WSS Service that is          */
   /* registered with a call to WSS_Initialize_XXX().  This function    */
   /* returns the non-zero number of attributes that are contained in a */
   /* WSS Server or zero on failure.                                    */
   /* * NOTE * This function may be used to determine the attribute     */
   /*          handle range for WSS so that the ServiceHandleRange      */
   /*          parameter of the WSS_Initialize_Service_Handle_Range()   */
   /*          can be configured to register WSS in a specified         */
   /*          attribute handle range in GATT.                          */
   /* * NOTE * Since WSS has a fixed number of mandatory attributes, the*/
   /*          value returned from this function will ALWAYS be the     */
   /*          same.                                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI WSS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_WSS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for setting the supported BM*/
   /* features on the specified WSS Instance.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* WSS_Initialize_XXX().  The final parameter is the supported       */
   /* features to set for the specified WSS Instance.  This function    */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * The SupportedFeatures parameter is a bitmask made up of  */
   /*          bits of the form WSS_WEIGHT_SCALE_FEATURE_FLAG_XXX.      */
   /* * NOTE * This function MUST be called after the WSS service is    */
   /*          registered with a successful call to WSS_Initialize_XXX()*/
   /*          in order to set the default features of the WSS server.  */
BTPSAPI_DECLARATION int BTPSAPI WSS_Set_Weight_Scale_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Set_Weight_Scale_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);
#endif

   /* The following function is responsible for querying the WSS        */
   /* Features on the specified WSS Instance.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* WSS_Initialize_XXX().  The final parameter is a pointer to return */
   /* the WSS Features for the specified WSS Instance.  This function   */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * The SupportedFeatures parameter is a bitmask made up of  */
   /*          bits of the form WSS_WEIGHT_SCALE_FEATURE_FLAG_XXX.      */
BTPSAPI_DECLARATION int BTPSAPI WSS_Query_Weight_Scale_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Query_Weight_Scale_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);
#endif

   /* /The following function is responsible for responding to a WSS    */
   /* Weight Measurement Characteristic's Client Characteristic         */
   /* Configuration descriptor (CCCD) read request.  The first parameter*/
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* WSS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The final       */
   /* parameter is the ClientConfiguration and is the current Client    */
   /* Characteristic Configuration value to send to the WSS Client if   */
   /* the request has been accepted.  This function returns a zero if   */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          WSS_ERROR_CODE_XXX from WSSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request is rejected the ClientConfiguration       */
   /*          parameter will be IGNORED.                               */
BTPSAPI_DECLARATION int BTPSAPI WSS_Read_Weight_Measurement_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Read_Weight_Measurement_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to a WSS     */
   /* Weight Measurement Characteristic's Client Characteristic         */
   /* Configuration descriptor (CCCD) write request.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to WSS_Initialize_XXX().  The third parameter is the GATT         */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* final parameter is the ClientConfiguration and is the current     */
   /* Client Characteristic Configuration value to send to the WSS      */
   /* Client if the request has been accepted.  This function returns a */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          WSS_ERROR_CODE_XXX from WSSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI WSS_Write_Weight_Measurement_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Write_Weight_Measurement_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending WSS Weight      */
   /* Measurement indications to a specified remote device.  The first  */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to WSS_Initialize_XXX().  The third parameter is the ConnectionID */
   /* of the remote device to send the indication to.  The final        */
   /* parameter is the WSS Weight Measurement to indicate.  This        */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the Weight Measurement CCCD has been configured for      */
   /*          indications by the WSS Client this indication is intended*/
   /*          for.  An indication SHOULD NOT be sent if this is not the*/
   /*          case.                                                    */
BTPSAPI_DECLARATION int BTPSAPI WSS_Indicate_Weight_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, WSS_Weight_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Indicate_Weight_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, WSS_Weight_Measurement_Data_t *MeasurementData);
#endif

   /* WSS Client API.                                                   */

   /* The following function is responsible for parsing a buffer        */
   /* received from a remote WSS Server and decoding a specified number */
   /* of weight measurements.  The first parameter is the length of the */
   /* Value returned by the remote WSS Server.  The second parameter is */
   /* a pointer to the Value returned by the remote WSS Server.  The    */
   /* final parameter is a pointer to store the decoded WSS Weight      */
   /* Measurement data.  This function returns a zero if successful or a*/
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI WSS_Decode_Weight_Measurement(unsigned int ValueLength, Byte_t *Value, WSS_Weight_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_WSS_Decode_Weight_Measurement_t)(unsigned int ValueLength, Byte_t *Value, WSS_Weight_Measurement_Data_t *MeasurementData);
#endif

#endif
