/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/****< bcsapi.h >**************************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BCSAPI - Qualcomm Technologies Bluetooth Body Composition Service (GATT   */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Michael Rougeux                                                  */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/20/15  M. Rougeux     Initial creation.                               */
/******************************************************************************/
#ifndef __BCSAPIH__
#define __BCSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "BCSTypes.h"      /* Body Composition Service Types/Constants.       */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BCS_ERROR_INVALID_PARAMETER                            (-1000)
#define BCS_ERROR_INVALID_BLUETOOTH_STACK_ID                   (-1001)
#define BCS_ERROR_INSUFFICIENT_RESOURCES                       (-1002)
#define BCS_ERROR_INSUFFICIENT_BUFFER_SPACE                    (-1003)
#define BCS_ERROR_SERVICE_ALREADY_REGISTERED                   (-1004)
#define BCS_ERROR_INVALID_INSTANCE_ID                          (-1005)
#define BCS_ERROR_MALFORMATTED_DATA                            (-1006)
#define BCS_ERROR_BODY_COMPOSITON_MEASUREMENT_FORMAT_FAILED    (-1007)

   /* The following structure contains the handles that will need to be */
   /* cached by a BCS client in order to only do service discovery      */
   /* once.                                                             */
typedef struct _tagBCS_Client_Information_t
{
   Word_t  BC_Feature_Handle;
   Word_t  BC_Measurement_Handle;
   Word_t  BC_Measurement_CCCD_Handle;
} BCS_Client_Information_t;

#define BCS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(BCS_Client_Information_t))

   /* The following structure contains the Client Characteristic        */
   /* Configuration descriptor that will need to be cached by a BCS     */
   /* Server for each BCS Client that connects to it.                   */
typedef struct _tagBCS_Server_Information_t
{
   Word_t  BC_Measurement_CCCD;
} BCS_Server_Information_t;

#define BCS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(BCS_Server_Information_t))

   /* The following structure represents the format of a BCS Date/Time  */
   /* value. This is used to represent the Date/Time which contains     */
   /* the Day/Month/Year and Hours:Minutes:Second data.                 */
   /* * NOTE * A value of 0 for the year, month or day fields shall not */
   /*          be used.                                                 */
typedef struct _tagBCS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} BCS_Date_Time_Data_t;

#define BCS_DATE_TIME_DATA_SIZE                          (sizeof(BCS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date Time is valid.  The only parameter to this  */
   /* function is the BCS_Date_Time_Data_t structure to validate.  This */
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
#define BCS_DATE_TIME_VALID(_x)                          ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following structure contains Body Composition Measurement     */
   /* data that is passed to the function that builds the Body          */
   /* Composition Measurement packet on the Server, and also the        */
   /* function that decodes the Body Composition Measurement client on  */
   /* the packet once indicated by the server.                          */
   /* * NOTE * If BCS_BC_MEASUREMENT_FLAG_MEASUREMENT_UNITS_SI Flag is  */
   /*          set, then units must be measured in SI.                  */
   /* * NOTE * If BCS_BC_MEASUREMENT_FLAG_MEASUREMENT_UNITS_IMPERIAL    */
   /*          Flag is set, then units must be measured in Imperial.    */
   /* * NOTE * If BCS_BC_MEASUREMENT_FLAG_TIME_STAMP_PRESENT_PRESENT    */
   /*          Flag is set, then a valid value must be given for        */
   /*          TimeStamp (non-zero Year, Month, and Day).               */
   /* * NOTE * If BCS_BC_MEASUREMENT_FLAG_USER_ID_PRESENT_PRESENT Flag  */
   /*          is set, then a valid value must be given for UserID; the */
   /*          special value BCS_BC_UNKNOWN_USER_ID may also be set. In */
   /*          addition, BCS_BC_FEATURE_FLAG_MULTIPLE_USERS_SUPPORTED   */
   /*          must be set in the service's features to use this field. */
   /* * NOTE * If BCS_BC_MEASUREMENT_FLAG_TIME_STAMP_PRESENT_PRESENT    */
   /*          Flag is set, then a valid value must be given for        */
   /*          TimeStamp (non-zero Year, Month, and Day).               */
typedef struct _tagBCS_BC_Measurement_Data_t
{
   Word_t               Flags;
   Word_t               BodyFatPercentage;
   BCS_Date_Time_Data_t TimeStamp;
   Byte_t               UserID;
   Word_t               BasalMetabolism;
   Word_t               MusclePercentage;
   Word_t               MuscleMass;
   Word_t               FatFreeMass;
   Word_t               SoftLeanMass;
   Word_t               BodyWaterMass;
   Word_t               Impedance;
   Word_t               Weight;
   Word_t               Height;
} BCS_BC_Measurement_Data_t;

#define BCS_BC_MEASUREMENT_DATA_SIZE                     (sizeof(BCS_BC_Measurement_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* BCS Service. These are used to determine the type of each event   */
   /* generated and to ensure the proper union element is accessed for  */
   /* the BCS_Event_Data_t structure.                                   */
typedef enum
{
   etBCS_Server_Read_CCCD_Request,
   etBCS_Server_Write_CCCD_Request,
   etBCS_Confirmation_Data
} BCS_Event_Type_t;

   /* The following is dispatched to a BCS Server when a BCS Client is  */
   /* attempting to read the BC Measurement's Client Characteristic     */
   /* Configuration descriptor.  The ConnectionID and RemoteDevice      */
   /* identify the Client that is making the request.  The TransactionID*/
   /* specifies the TransactionID of the request, this can be used when */
   /* responding to the request using the                               */
   /* BCS_Client_Configuration_Read_Response() API function.            */
typedef struct _tagBCS_Read_CCCD_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   unsigned int            TransactionID;
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDevice;
} BCS_Read_CCCD_Request_Data_t;

#define BCS_READ_CCCD_REQUEST_DATA_SIZE                 (sizeof(BCS_Read_CCCD_Request_Data_t))

   /* The following is dispatched to a BCS Server when a BCS Client     */
   /* attempts to write the BC Measurement's Client Characteristic   */
   /* Configuration descriptor.  The ConnectionID and RemoteDevice      */
   /* identify the Client that is making the request.  The              */
   /* ClientConfiguration field specifies the new Client Characteristic */
   /* Configuration value.                                              */
typedef struct _tagBCS_Write_CCCD_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   unsigned int            TransactionID;
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDevice;
   Word_t                  ClientConfiguration;
} BCS_Write_CCCD_Request_Data_t;

#define BCS_WRITE_CCCD_REQUEST_DATA_SIZE                (sizeof(BCS_Write_CCCD_Request_Data_t))

   /* The following is dispatched to a BCS Server when a BCS Client     */
   /* confirms an outstanding indication.  The ConnectionID and         */
   /* RemoteDevice identify the Client that has confirmed the           */
   /* indication.  The BytesWritten field indicates the number of bytes */
   /* that were successfully written in the outstanding indication.     */
typedef struct _tagBCS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
   Word_t                 BytesWritten;
} BCS_Confirmation_Data_t;

#define BCS_CONFIRMATION_DATA_SIZE                      (sizeof(BCS_Confirmation_Data_t))

   /* The following structure holds all BCS Service Event Data. This    */
   /* structure is received for each event generated. The               */
   /* Event_Data_Type member is used to determine the appropriate union */
   /* member element to access the contained data. The Event_Data_Size  */
   /* member contains the total size of the data contained in this      */
   /* event.                                                            */
typedef struct _tagBCS_Event_Data_t
{
   BCS_Event_Type_t                  Event_Data_Type;
   Word_t                            Event_Data_Size;
   union
   {
      BCS_Read_CCCD_Request_Data_t  *BCS_Read_CCCD_Request_Data;
      BCS_Write_CCCD_Request_Data_t *BCS_Write_CCCD_Request_Data;
      BCS_Confirmation_Data_t       *BCS_Confirmation_Data;
   } Event_Data;
} BCS_Event_Data_t;

#define BCS_EVENT_DATA_SIZE                              (sizeof(BCS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a BCS Service Event Callback. This function will be called        */
   /* whenever a BCS Service Event occurs that is associated with the   */
   /* specified Bluetooth Stack ID. This function passes to the caller  */
   /* the Bluetooth Stack ID, the BCS Event Data that occurred, and the */
   /* BCS Service Event Callback Parameter that was specified when this */
   /* Callback was installed. The caller is free to use the contents of */
   /* the BCS Service Event Data ONLY in the context of this callback.  */
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
   /* (this argument holds anyway because another BCS Service Event     */
   /* will not be processed while this function call is outstanding).   */
   /* * NOTE * This function MUST NOT block and wait for events that    */
   /*          can only be satisfied by receiving BCS Service Event     */
   /*          Packets. A Deadlock WILL occur because NO BCS Event      */
   /*          Callbacks will be issued while this function is          */
   /*          currently outstanding.                                   */
typedef void (BTPSAPI *BCS_Event_Callback_t)(unsigned int BluetoothStackID, BCS_Event_Data_t *BCS_Event_Data, unsigned long CallbackParameter);

   /* BCS Server API.                                                   */

   /* The following function is responsible for opening a BCS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the BCS Service Flags  */
   /* (BCS_SERVICE_FLAGS_XXX) found in AIOSTypes.h.  These flags MUST be*/
   /* used to register the BCS service for the correct transport.  The  */
   /* third parameter is the Callback function to call when an event    */
   /* occurs on this Server Port.  The fourth parameter is a            */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered BCS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 BCS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * The GATT Service Flags may be BCS_SERVICE_FLAGS_LE,      */
   /*          BCS_SERVICE_FLAGS_BR_EDR, or BCS_SERVICE_FLAGS_DUAL_MODE.*/
   /*          These are defined in BCSTypes.h.                         */
   /* * NOTE * If BR/EDR is not supported by the Bluetopia configuration*/
   /*          then the BCS_SERVICE_FLAGS_BR_EDR bit will be cleared in */
   /*          the Flags parameter and the SDP Record will not be       */
   /*          registered.  If the BCS_SERVICE_FLAGS_LE bit is also set,*/
   /*          then this function will still return success even though */
   /*          the service was not registered for BR/EDR.               */
BTPSAPI_DECLARATION int BTPSAPI BCS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Flags, BCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Flags, BCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a BCS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the BCS Service Flags  */
   /* (BCS_SERVICE_FLAGS_XXX) found in AIOSTypes.h.  These flags MUST be*/
   /* used to register the BCS service for the correct transport.  The  */
   /* third parameter is the Callback function to call when an event    */
   /* occurs on this Server Port.  The fourth parameter is a            */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The fifth parameter is a      */
   /* pointer to store the GATT Service ID of the registered BCS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 BCS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * The GATT Service Flags may be BCS_SERVICE_FLAGS_LE,      */
   /*          BCS_SERVICE_FLAGS_BR_EDR, or BCS_SERVICE_FLAGS_DUAL_MODE.*/
   /*          These are defined in BCSTypes.h.                         */
   /* * NOTE * If BR/EDR is not supported by the Bluetopia configuration*/
   /*          then the BCS_SERVICE_FLAGS_BR_EDR bit will be cleared in */
   /*          the Flags parameter and the SDP Record will not be       */
   /*          registered.  If the BCS_SERVICE_FLAGS_LE bit is also set,*/
   /*          then this function will still return success even though */
   /*          the service was not registered for BR/EDR.               */
BTPSAPI_DECLARATION int BTPSAPI BCS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Flags, BCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Flags, BCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened BCS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfull call to           */
   /* BCS_Initialize_XXX().  This function returns zero if successful or*/
   /* a negative error code.                                            */
BTPSAPI_DECLARATION int BTPSAPI BCS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI BCS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_BCS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* BCS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to BCS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI BCS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the BCS Service that is          */
   /* registered with a call to BCS_Initialize_XXX().  This function    */
   /* returns the non-zero number of attributes that are contained in a */
   /* BCS Server or zero on failure.                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI BCS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_BCS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for setting the supported BM*/
   /* features on the specified BCS Instance.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* BCS_Initialize_XXX().  The final parameter is the supported       */
   /* features to set for the specified BCS Instance.  This function    */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * The SupportedFeatures parameter is a bitmask made up of  */
   /*          bits of the form BCS_BC_FEATURE_FLAG_XXX.                */
   /* * NOTE * This function MUST be called after the BCS service is    */
   /*          registered with a successful call to BCS_Initialize_XXX()*/
   /*          in order to set the default features of the BCS server.  */
BTPSAPI_DECLARATION int BTPSAPI BCS_Set_BC_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Set_BC_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);
#endif

   /* The following function is responsible for querying the BCS        */
   /* Features on the specified BCS Instance.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* BCS_Initialize_XXX().  The final parameter is a pointer to return */
   /* the BCS Features for the specified BCS Instance.  This function   */
   /* returns zero if successful or a negative error code.              */
   /* * NOTE * The SupportedFeatures parameter is a bitmask made up of  */
   /*          bits of the form BCS_BC_FEATURE_FLAG_XXX.                */
BTPSAPI_DECLARATION int BTPSAPI BCS_Query_BC_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Query_BC_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);
#endif

   /* The following function is responsible for responding to a request */
   /* from a BCS Client to read the BC Measurement's Client             */
   /* Characteristic Configuration descriptor.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* BCS_Initialize_XXX().  The third is the Transaction ID of the     */
   /* request.  The fourth parameter is the ErrorCode to indicate the   */
   /* type of response that will be sent.  The final parameter contains */
   /* the Client Configuration to send to the remote device.  This      */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          BCS_ERROR_CODE_XXX from BCSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The Client_Configuration parameter is only REQUIRED if   */
   /*          the ErrorCode parameter is BCS_ERROR_CODE_SUCCESS.       */
   /*          Otherwise it will be ignored.                            */
BTPSAPI_DECLARATION int BTPSAPI BCS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t Client_Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t Client_Configuration);
#endif

   /* The following function is responsible for responding to a request */
   /* from a BCS Client to write the BC Measurement's Client            */
   /* Characteristic Configuration descriptor.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* BCS_Initialize_XXX().  The third is the Transaction ID of the     */
   /* request.  The final parameter is the ErrorCode to indicate the    */
   /* type of response that will be sent.  This function returns a zero */
   /* if successful or a negative return error code if an error occurs. */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          BCS_ERROR_CODE_XXX from BCSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI BCS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending a Body          */
   /* Composition Measurement indication to a specified remote device.  */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to BCS_Initialize_Service().  The third parameter */
   /* is the ConnectionID of the remote device to send the indication   */
   /* to.  The final parameter is the Body Composition Measurement data */
   /* to indicate.  This function returns a zero if successful or a     */
   /* negative return error code if an error occurs.                    */
   /* ** NOTE ** This function will determine if the BC_Measurement     */
   /*            field needs to be split and sent in multiple           */
   /*            indications.  If this is the case, this function will  */
   /*            set the                                                */
   /*            BCS_BC_MEASUREMENT_FLAG_MULTIPLE_PACKET_MEASUREMENT    */
   /*            flag in the Flags field of the                         */
   /*            BCS_BC_Measurement_Data_t structure and will send      */
   /*            multiple indications for the BC Measurement.           */
   /* * NOTE * If the BC_Measurement's Flags Body Fat Percentage field  */
   /*          is set to BCS_BC_MEASUREMENT_UNSUCCESSFUL, then all bits */
   /*          except                                                   */
   /*          BCS_BC_MEASUREMENT_FLAG_MEASUREMENT_UNITS_IMPERIAL,      */
   /*          BCS_BC_MEASUREMENT_FLAG_TIME_STAMP_PRESENT, and          */
   /*          BCS_BC_MEASUREMENT_FLAG_USER_ID_PRESENT will be cleared  */
   /*          if set.  The Time Stamp and User ID should not be        */
   /*          included in an unsuccessful measurement.                 */
BTPSAPI_DECLARATION int BTPSAPI BCS_Indicate_Body_Composition_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, BCS_BC_Measurement_Data_t *BC_Measurement);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Indicate_Body_Composition_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, BCS_BC_Measurement_Data_t *BC_Measurement);
#endif

   /* BCS Client API.                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote BCS Server interpreting it as a Body Composition    */
   /* Measurement characteristic.  The first parameter is the length of */
   /* the value returned by the remote BCS Server.  The second parameter*/
   /* is a pointer to the data returned by the remote BCS Server.  The  */
   /* final parameter is a pointer to store the parsed Body Composition */
   /* Measurement value.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
   /* * NOTE * If the BC_Measurement's Flags Body Fat Percentage field  */
   /*          is set to BCS_BC_MEASUREMENT_UNSUCCESSFUL, then all bits */
   /*          except                                                   */
   /*          BCS_BC_MEASUREMENT_FLAG_MEASUREMENT_UNITS_IMPERIAL,      */
   /*          BCS_BC_MEASUREMENT_FLAG_TIME_STAMP_PRESENT, and          */
   /*          BCS_BC_MEASUREMENT_FLAG_USER_ID_PRESENT will be cleared  */
   /*          if set.  The Time Stamp and User ID should not be        */
   /*          included in an unsuccessful measurement.                 */
BTPSAPI_DECLARATION int BTPSAPI BCS_Decode_Body_Composition_Measurement(unsigned int ValueLength, Byte_t *Value, BCS_BC_Measurement_Data_t *BC_Measurement);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BCS_Decode_Body_Composition_Measurement_t)(unsigned int ValueLength, Byte_t *Value, BCS_BC_Measurement_Data_t *BC_Measurement);
#endif

#endif
