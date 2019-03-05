/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< plxsapi.h >************************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PLXSAPI - Qualcomm Technologies Bluetooth Pulse Oximeter Service (GATT    */
/*            based) Type Definitions, Prototypes, and Constants.             */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/06/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __PLXSAPIH__
#define __PLXSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "PLXSTypes.h"     /* Pulse Oximeter Service Types/Constants          */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define PLXS_ERROR_INVALID_PARAMETER                     (-1000)
#define PLXS_ERROR_INVALID_BLUETOOTH_STACK_ID            (-1001)
#define PLXS_ERROR_INSUFFICIENT_RESOURCES                (-1002)
#define PLXS_ERROR_SERVICE_ALREADY_REGISTERED            (-1003)
#define PLXS_ERROR_INVALID_INSTANCE_ID                   (-1004)
#define PLXS_ERROR_MALFORMATTED_DATA                     (-1005)
#define PLXS_ERROR_INSUFFICIENT_BUFFER_SPACE             (-1006)
#define PLXS_ERROR_MEASUREMENT_NOT_SUPPORTED             (-1007)
#define PLXS_ERROR_INVALID_CCCD_TYPE                     (-1008)
#define PLXS_ERROR_INVALID_ATTRIBUTE_HANDLE              (-1009)
#define PLXS_ERROR_RACP_NOT_SUPPORTED                    (-1010)
#define PLXS_ERROR_INVALID_RACP_RESPONSE_OP_CODE         (-1011)

   /* The following structure defines the information needed to         */
   /* initialize the PLXS Server.                                       */
   /* * NOTE * The Spot_Check_Measurement and Continuous_Measurement    */
   /*          fields indicate whether the PLXS Server will support     */
   /*          these Characteristics.  At least one of these fields MUST*/
   /*          be supported.                                            */
   /* * NOTE * If the Spot Check Measurement Characteristic is supported*/
   /*          a Client Characteristic Configuration Descriptor (CCCD), */
   /*          will automatically be included so that a PLXS Client can */
   /*          enable indications.                                      */
   /* * NOTE * If the Continuous_Measurement Characteristic is supported*/
   /*          a Client Characteristic Configuration Descriptor (CCCD), */
   /*          will automatically be included so that a PLXS Client can */
   /*          enable notifications.                                    */
   /* * NOTE * The Measurement_Storage field indicates if the PLXS      */
   /*          Server is capable of storing Spot Check Measurements.    */
   /*          The Spot_Check_Measurement field MUST be TRUE, since only*/
   /*          Spot-Check Measurements may be stored.  Otherwise this   */
   /*          field will be ignored if Spot_Check_Measurement is FALSE.*/
   /* * NOTE * If Measurement_Storage is TRUE, and all above            */
   /*          requirements have been met, then a Record Access Control */
   /*          Point (RACP) Characteristic will automatically be        */
   /*          included.  A Client Characteristic Configuration         */
   /*          Descriptor (CCCD), will automatically be included so that*/
   /*          a PLXS Client can enable indications.                    */
typedef struct _tagPLXS_Initialize_Data_t
{
   Boolean_t Spot_Check_Measurement;
   Boolean_t Continuous_Measurement;
   Boolean_t Measurement_Storage;
} PLXS_Initialize_Data_t;

#define PLXS_INITIALIZE_DATA_SIZE                        (sizeof(PLXS_Initialize_Data_t))

   /* The following structure contains the handles that will need to be */
   /* cached by a PLXS client in order to only do service discovery     */
   /* once.                                                             */
typedef struct _tagPLXS_Client_Information_t
{
   Word_t Spot_Check_Measurement;
   Word_t Spot_Check_Measurement_CCCD;
   Word_t Continuous_Measurement;
   Word_t Continuous_Measurement_CCCD;
   Word_t PLX_Features;
   Word_t Record_Access_Control_Point;
   Word_t Record_Access_Control_Point_CCCD;
} PLXS_Client_Information_t;

#define PLXS_CLIENT_INFORMATION_DATA_SIZE                (sizeof(PLXS_Client_Information_t))

   /* The following structure contains the Client Characteristic        */
   /* Configuration descriptor that will need to be cached by a PLXS    */
   /* Server for each PLXS Client that connects to it.                  */
typedef struct _tagPLXS_Server_Information_t
{
   Word_t Spot_Check_Measurement_CCCD;
   Word_t Continuous_Measurement_CCCD;
   Word_t Record_Access_Control_Point_CCCD;
} PLXS_Server_Information_t;

#define PLXS_SERVER_INFORMATION_DATA_SIZE                (sizeof(PLXS_Server_Information_t))

   /* The following represents the format for the PLXS INT24 data type. */
typedef struct _tagPLXS_INT24_Data_t
{
   Word_t Lower;
   Byte_t Upper;
} PLXS_INT24_Data_t;

#define PLXS_INT24_DATA_SIZE                             (sizeof(PLXS_INT24_Data_t))

   /* The following enumeration defines the PLXS Client Characteristic  */
   /* Configuration Descriptor (CCCD) types.                            */
typedef enum
{
   pcdSpotCheck,
   pcdContinuous,
   pcdRACP,
   pcdServiceChanged
} PLXS_CCCD_Type_t;

   /* The following structure represents the PLXS Features.  This       */
   /* structure is used to identify the supported features of the PLXS  */
   /* Server.  This structure is also used to identify the supported    */
   /* Measurement Status bits and Device and Sensor Status bits that may*/
   /* optionally be included in PLXS Measurements.                      */
   /* * NOTE * These fields contain global features for the PLXS Server.*/
   /*          That is if a feature or option is not supported, then    */
   /*          that feature CANNOT be used for PLXS Measurements.       */
typedef struct _tagPLXS_Features_Data_t
{
   Word_t            Support_Features;
   Word_t            Measurement_Status_Support;
   PLXS_INT24_Data_t Device_And_Sensor_Status_Support;
} PLXS_Features_Data_t;

#define PLXS_FEATURES_DATA_SIZE                          (sizeof(PLXS_Features_Data_t))

   /* The following represents the format for the PLXS Date Time data   */
   /* type.                                                             */
   /* * NOTE * The Year field is defined by the Gregorian calendar and  */
   /*          MUST be between 1582-9999.                               */
   /* * NOTE * The Month field is defined by the Gregorian calendar and */
   /*          MUST be between 0-12.  A value of zero means unknown.    */
   /* * NOTE * The Day field is defined by the Gregorian calendar and   */
   /*          MUST be between 0-31.  A vlaue of zero means unknown.    */
   /* * NOTE * The Hours field is the number of hours past midnight and */
   /*          MUST be between 0-23.                                    */
   /* * NOTE * The Minutes field is the number of minutes since the     */
   /*          start of the hour and MUST be between 0-59.              */
   /* * NOTE * The Seconds field is the number of seconds since the     */
   /*          start of the minute and MUST be between 0-59.            */
typedef struct _tagPLXS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
}  PLXS_Date_Time_Data_t;

#define PLXS_DATE_TIME_DATA_SIZE                         (sizeof(PLXS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date Time is valid.  The only parameter to this  */
   /* function is the PLXS_Date_Time_Data_t structure to validate.  This*/
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
#define PLXS_DATE_TIME_VALID(_x)                         ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following enumeration defines the PLXS Measurement types.     */
typedef enum
{
   pctSpotCheck,
   pctContinuous
} PLXS_Measurement_Type_t;

   /* The following structure represents the format for a PLXS          */
   /* Spot-Check Measurement.                                           */
   /* * NOTE * The Flags, SpO2, and PR fields are MANDATORY fields,     */
   /*          however the remaining fields may be optionally included  */
   /*          if specfied by the Flags field.                          */
   /* * NOTE * In order to use some optional fields and their bits      */
   /*          values, the PLXS Server MUST support the feature (See the*/
   /*          PLXS_Features_Data_t structure for more information).    */
   /*          These are global requirements that affect each Spot-Check*/
   /*          Measurement.                                             */
   /* * NOTE * The Flags field is a bit mask that has the form          */
   /*          PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_XXX and is used to     */
   /*          include optional fields and specify if the Device Clock  */
   /*          has been set.  This allows each PLXS Spot-Check          */
   /*          Measurement to include optional fields independently from*/
   /*          other Spot-Check Measurements                            */
typedef struct _tagPLXS_Spot_Check_Measurement_Data_t
{
   Byte_t                Flags;
   Word_t                SpO2;
   Word_t                PR;
   PLXS_Date_Time_Data_t Timestamp;
   Word_t                Measurement_Status;
   PLXS_INT24_Data_t     Device_And_Sensor_Status;
   Word_t                Pulse_Amplitude_Index;
} PLXS_Spot_Check_Measurement_Data_t;

#define PLXS_SPOT_CHECK_MEASUREMENT_DATA_SIZE            (sizeof(PLXS_Spot_Check_Measurement_Data_t))

   /* The following structure represents the format for a PLXS          */
   /* Continuous Measurement.                                           */
   /* * NOTE * The Flags, SpO2, and PR fields are MANDATORY fields,     */
   /*          however the remaining fields may be optionally included  */
   /*          if specified by the Flags field.                         */
   /* * NOTE * In order to use some optional fields and their bits      */
   /*          values, the PLXS Server MUST support the feature (See the*/
   /*          PLXS_Features_Data_t structure for more information).    */
   /*          These are global requirements that affect each Continuous*/
   /*          Measurement.                                             */
   /* * NOTE * The Flags field is a bit mask that has the form          */
   /*          PLXS_CONTINUOUS_MEASUREMENT_FLAGS_XXX and is used to     */
   /*          include optional fields.  This allows each PLXS          */
   /*          Continuous Measurement to include optional fields        */
   /*          independently from other Continuous Measurements.        */
typedef struct _tagPLXS_Continuous_Measurement_Data_t
{
   Byte_t            Flags;
   Word_t            SpO2_Normal;
   Word_t            PR_Normal;
   Word_t            SpO2_Fast;
   Word_t            PR_Fast;
   Word_t            SpO2_Slow;
   Word_t            PR_Slow;
   Word_t            Measurement_Status;
   PLXS_INT24_Data_t Device_And_Sensor_Status;
   Word_t            Pulse_Amplitude_Index;
} PLXS_Continuous_Measurement_Data_t;

#define PLXS_CONTINUOUS_MEASUREMENT_DATA_SIZE            (sizeof(PLXS_Continuous_Measurement_Data_t))

   /* The following enumeration defines the valid values that may be set*/
   /* for the Op_Code field of the PLXS_RACP_Request_Data_t structure.  */
typedef enum
{
   rrtReportStoredRecordsRequest    = PLXS_RACP_OPCODE_REPORT_STORED_RECORDS,
   rrtDeleteStoredRecordsRequest    = PLXS_RACP_OPCODE_DELETE_STORED_RECORDS,
   rrtAbortOperationRequest         = PLXS_RACP_OPCODE_ABORT_OPERATION,
   rrtNumberOfStoredRecordsRequest  = PLXS_RACP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS
} PLXS_RACP_Request_Type_t;

   /* The following enumeration defines the valid values that may be set*/
   /* for the Operator field of the PLXS_RACP_Request_Data_t structure. */
typedef enum
{
   rotNull       = PLXS_RACP_OPERATOR_NULL,
   rotAllRecords = PLXS_RACP_OPERATOR_ALL_RECORDS,
} PLXS_RACP_Operator_Type_t;

   /* The following structure defines the Record Access Control Point   */
   /* (RACP) Request data that may be received in an RACP Write request.*/
typedef struct _tagPLXS_RACP_Request_Data_t
{
   PLXS_RACP_Request_Type_t  Op_Code;
   PLXS_RACP_Operator_Type_t Operator;
} PLXS_RACP_Request_Data_t;

#define PLXS_RACP_REQUEST_DATA_SIZE                      (sizeof(PLXS_RACP_Request_Data_t))

   /* The following enumeration defines the valid values that may be set*/
   /* for the Op_Code field of the PLXS_RACP_Response_Data_t structure. */
typedef enum
{
   rrotNumberOfStoredRecordsResponse = PLXS_RACP_OPCODE_NUMBER_OF_STORED_RECORDS_RESPONSE,
   rrotResponseOpCode                = PLXS_RACP_OPCODE_RESPONSE_CODE
} PLXS_RACP_Response_Type_t;

   /* The following enumeration defines the valid values that may be set*/
   /* for the Operand field of the PLXS_RACP_Response_Data_t structure  */
   /* if the Response_Op_Code field is NOT set to                       */
   /* rrotNumberOfStoredRecordsResponse or the RACP Request was NOT     */
   /* successful.                                                       */
typedef enum
 {
   rcvPLXSSuccess               = PLXS_RACP_RESPONSE_CODE_VALUE_SUCCESS,
   rcvPLXSOpCodeNotSupported    = PLXS_RACP_RESPONSE_CODE_VALUE_OPCODE_NOT_SUPPORTED,
   rcvPLXSInvalidOperator       = PLXS_RACP_RESPONSE_CODE_VALUE_INVALID_OPERATOR,
   rcvPLXSOperatorNotSupported  = PLXS_RACP_RESPONSE_CODE_VALUE_OPERATOR_NOT_SUPPORTED,
   rcvPLXSInvalidOperand        = PLXS_RACP_RESPONSE_CODE_VALUE_INVALID_OPERAND,
   rcvPLXSNoRecordFound         = PLXS_RACP_RESPONSE_CODE_VALUE_NO_RECORDS_FOUND,
   rcvPLXSAbortUnsuccessful     = PLXS_RACP_RESPONSE_CODE_VALUE_ABORT_UNSUCCESSFUL,
   rcvPLXSProcedureNotCompleted = PLXS_RACP_RESPONSE_CODE_VALUE_PROCEDURE_NOT_COMPLETED,
   rcvPLXSOperandNotSupported   = PLXS_RACP_RESPONSE_CODE_VALUE_OPERAND_NOT_SUPPORTED,
} PLXS_RACP_Response_Code_Value_t;

   /* The following structure defines the Record Access Control Point   */
   /* (RACP) Response data that may be sent in a indication to a PLXS   */
   /* Client that sent an RACP request.                                 */
   /* * NOTE * The Request_Op_Code field is only valid if the Operand is*/
   /*          Response_Code.                                           */
typedef struct _tagPLXS_RACP_Response_Data_t
{
   PLXS_RACP_Response_Type_t Response_Op_Code;
   PLXS_RACP_Operator_Type_t Operator;
   PLXS_RACP_Request_Type_t  Request_Op_Code;
   union
   {
      Word_t                          Number_Of_Stored_Records;
      PLXS_RACP_Response_Code_Value_t Response_Code;
   } Operand;
} PLXS_RACP_Response_Data_t;

#define PLXS_RACP_RESPONSE_DATA_SIZE                     (sizeof(PLXS_RACP_Response_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* PLXS for the PLXS Server.  These are used to determine the type of*/
   /* each event generated, and to ensure the proper union element is   */
   /* accessed for the PLXS_Event_Data_t structure.                     */
typedef enum _tagPLXS_Event_Type_t
{
   etPLXS_Server_Read_Features_Request,
   etPLXS_Server_Write_RACP_Request,
   etPLXS_Server_Read_CCCD_Request,
   etPLXS_Server_Write_CCCD_Request,
   etPLXS_Server_Confirmation
} PLXS_Event_Type_t;

   /* The following PLXS Server Event is dispatched to a PLXS Server    */
   /* when a PLXS Client has requested to read the PLXS Features        */
   /* Characteristic.  The InstanceID identifies the PLXS Instance that */
   /* dispatched the event.  The ConnectionID identifies the GATT       */
   /* Connection Identifier for the request.  The ConnectionType        */
   /* identifies the GATT Connection type.  The TransactionID identifies*/
   /* the GATT Transaction Identifier for the request.  The RemoteDevice*/
   /* identifies the Bluetooth Address of the PLXS Client.              */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the                                    */
   /*          PLXS_Read_Features_Request_Response() function.  Some of */
   /*          the fields below are needed for the response.            */
typedef struct _tagPLXS_Read_Features_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
} PLXS_Read_Features_Request_Data_t;

#define PLXS_READ_FEATURES_REQUEST_DATA_SIZE             (sizeof(PLXS_Read_Features_Request_Data_t))

   /* The following PLXS Server Event is dispatched to a PLXS Server    */
   /* when a PLXS Client has requested to write the Record Access       */
   /* Control Point (RACP) Characteristic.  The InstanceID identifies   */
   /* the PLXS Instance that dispatched the event.  The ConnectionID    */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* PLXS Client.  The RequestData represents the RACP request data    */
   /* that has been received from the PLXS Client.                      */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the PLXS_RACP_Request_Response()       */
   /*          function.  Some of the fields below are needed for the   */
   /*          response.  This response does not indicate if the RACP   */
   /*          request was successful.  It simply indicates that the    */
   /*          request has been accepted and is being processed.  Once  */
   /*          the RACP request has been processed an indication MUST be*/
   /*          sent for the result of the RACP Procedure.               */
typedef struct _tagPLXS_Write_RACP_Request_Data_t
{
   unsigned int             InstanceID;
   unsigned int             ConnectionID;
   GATT_Connection_Type_t   ConnectionType;
   unsigned int             TransactionID;
   BD_ADDR_t                RemoteDevice;
   PLXS_RACP_Request_Data_t RequestData;
} PLXS_Write_RACP_Request_Data_t;

#define PLXS_WRITE_RACP_REQUEST_DATA_SIZE                (sizeof(PLXS_Write_RACP_Request_Data_t))

   /* The following PLXS Server Event is dispatched to a PLXS Server    */
   /* when a PLXS Client has requested to read a Client Characteristic  */
   /* Configuration descriptor (CCCD).  The InstanceID identifies the   */
   /* PLXS Instance that dispatched the event.  The ConnectionID        */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* PLXS Client.  The Type field is an enumeration that identifies the*/
   /* CCCD type that has been requested.                                */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the PLXS_Read_CCCD_Request_Response()  */
   /*          function.  Some of the fields below are needed for the   */
   /*          response.                                                */
typedef struct _tagPLXS_Read_CCCD_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   PLXS_CCCD_Type_t       Type;
} PLXS_Read_CCCD_Request_Data_t;

#define PLXS_READ_CCCD_REQUEST_DATA_SIZE                 (sizeof(PLXS_Read_CCCD_Request_Data_t))

   /* The following PLXS Server Event is dispatched to a PLXS Server    */
   /* when a PLXS Client has requested to write a Client Characteristic */
   /* Configuration descriptor (CCCD).  The InstanceID identifies the   */
   /* PLXS Instance that dispatched the event.  The ConnectionID        */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* PLXS Client.  The Type field is an enumeration that identifies the*/
   /* CCCD type that has been requested.  The Configuration field is the*/
   /* value for the CCCD that has been requested to be written.         */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the PLXS_Write_CCCD_Request_Response() */
   /*          function.  Some of the fields below are needed for the   */
   /*          response.                                                */
typedef struct _tagPLXS_Write_CCCD_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   PLXS_CCCD_Type_t       Type;
   Word_t                 Configuration;
} PLXS_Write_CCCD_Request_Data_t;

#define PLXS_WRITE_CCCD_REQUEST_DATA_SIZE                (sizeof(PLXS_Write_CCCD_Request_Data_t))

   /* The following is dispatched to a PLXS Server when a PLXS Client   */
   /* confirms an outstanding indication.  The InstanceID identifies the*/
   /* PLXS Instance that dispatched the event.  The ConnectionID        */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* PLXS Client.  The Status field contains the result of the         */
   /* confirmation.  The BytesWritten field indicates the number of     */
   /* bytes that were successfully indicated to the PLXS Client.        */
typedef struct _tagPLXS_Confirmation_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
   Byte_t                  Status;
   Word_t                  BytesWritten;
} PLXS_Confirmation_Data_t;

#define PLXS_CONFIRMATION_DATA_SIZE                      (sizeof(PLXS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all PLXS Service Event Data.  This structure is received  */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagPLXS_Event_Data_t
{
   PLXS_Event_Type_t  Event_Data_Type;
   Word_t             Event_Data_Size;
   union
   {
      PLXS_Read_Features_Request_Data_t *PLXS_Read_Features_Request_Data;
      PLXS_Write_RACP_Request_Data_t    *PLXS_Write_RACP_Request_Data;
      PLXS_Read_CCCD_Request_Data_t     *PLXS_Read_CCCD_Request_Data;
      PLXS_Write_CCCD_Request_Data_t    *PLXS_Write_CCCD_Request_Data;
      PLXS_Confirmation_Data_t          *PLXS_Confirmation_Data;
   } Event_Data;
} PLXS_Event_Data_t;

#define PLXS_EVENT_DATA_SIZE                             (sizeof(PLXS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a PLXS Profile Event Receive Data Callback.  This function will be*/
   /* called whenever an PLXS Profile Event occurs that is associated   */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the PLXS Event Data that       */
   /* occurred and the PLXS Profile Event Callback Parameter that was   */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the PLXS Profile Event Data ONLY in the       */
   /* context of this callback.  If the caller requires the Data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another Data Buffer This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have to be       */
   /* re-entrant).It needs to be noted however, that if the same        */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another PLXS Profile Event will not be processed while this       */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving PLXS Profile Event  */
   /*            Packets.  A Deadlock WILL occur because NO PLXS Event  */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *PLXS_Event_Callback_t)(unsigned int BluetoothStackID, PLXS_Event_Data_t *PLXS_Event_Data, unsigned long CallbackParameter);

   /* PLXS Server API.                                                  */

   /* The following function is responsible for opening a PLXS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the PLXS Service Flags */
   /* (PLXS_SERVICE_FLAGS_XXX) from PLXSTypes.h.  These flags MUST be   */
   /* used to register the GATT service for the correct transport.  The */
   /* third parameter is a pointer to the PLXS_Initialize_Data_t        */
   /* structure that contains the information needed to initialize and  */
   /* configure the service.  The fourth parameter is the Callback      */
   /* function to call when an event occurs on this Server Port.  The   */
   /* fifth parameter is a user-defined callback parameter that will be */
   /* passed to the callback function with each event.  The final       */
   /* parameter is a pointer to store the GATT Service ID of the        */
   /* registered PLXS service.  This function returns the positive,     */
   /* non-zero, Instance ID or a negative error code.                   */
   /* * NOTE * The InitializeData parameter MUST be valid.  This API    */
   /*          will FAIL if the structure pointed to by the             */
   /*          InitializeData parameter is configured incorrectly.      */
   /*          Since there is too much information to cover here, please*/
   /*          see the PLXS_Initialize_Data_t structure in PLXSAPI.h for*/
   /*          more information about configuring this structure.       */
   /* * NOTE * Only 1 PLXS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, PLXS_Initialize_Data_t *InitializeData, PLXS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, PLXS_Initialize_Data_t *InitializeData, PLXS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a PLXS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the PLXS Service Flags */
   /* (PLXS_SERVICE_FLAGS_XXX) from PLXSTypes.h.  These flags MUST be   */
   /* used to register the GATT service for the correct transport.  The */
   /* third parameter is a pointer to the PLXS_Initialize_Data_t        */
   /* structure that contains the information needed to initialize and  */
   /* configure the service.  The fourth parameter is the Callback      */
   /* function to call when an event occurs on this Server Port.  The   */
   /* fifth parameter is a user-defined callback parameter that will be */
   /* passed to the callback function with each event.  The sixth       */
   /* parameter is a pointer to store the GATT Service ID of the        */
   /* registered PLXS service.  The final parameter is a pointer, that  */
   /* on input can be used to control the location of the service in the*/
   /* GATT database, and on ouput to store the service handle range.    */
   /* This function returns the positive, non-zero, Instance ID or a    */
   /* negative error code.                                              */
   /* * NOTE * The InitializeData parameter MUST be valid.  This API    */
   /*          will FAIL if the structure pointed to by the             */
   /*          InitializeData parameter is configured incorrectly.      */
   /*          Since there is too much information to cover here, please*/
   /*          see the PLXS_Initialize_Data_t structure in PLXSAPI.h for*/
   /*          more information about configuring this structure.       */
   /* * NOTE * Only 1 PLXS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, PLXS_Initialize_Data_t *InitializeData, PLXS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, PLXS_Initialize_Data_t *InitializeData, PLXS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previous PLXS */
   /* Server.  The first parameter is the Bluetooth Stack ID on which to*/
   /* close the server.  The second parameter is the InstanceID that was*/
   /* returned from a successful call to either of the                  */
   /* PLXS_Initialize_XXX() functions.  This function returns a zero if */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI PLXS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_PLXS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* PLXS_Suspend()).  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to PLXS_Suspend().  This    */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the PLXS Service that is         */
   /* registered with a call to either of the PLXS_Initialize_XXX()     */
   /* functions.  The first parameter is the Bluetooth Stack ID of the  */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to either of the PLXS_Initialize_XXX()     */
   /* functions.  This function returns the non-zero number of          */
   /* attributes that are contained in a PLXS Server or zero on failure.*/
BTPSAPI_DECLARATION unsigned int BTPSAPI PLXS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_PLXS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from a PLXS Client for a PLXS Characteristic's Client     */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to PLXS_Initialize_XXX().  The third parameter is the GATT        */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* final parameter is a pointer to the PLXS Features that will be    */
   /* sent to the PLXS Client is the request has been accepted.  This   */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          PLXS_ERROR_CODE_XXX from PLXSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The Features parameter is only REQUIRED if the ErrorCode */
   /*          parameter is PLXS_ERROR_CODE_SUCCESS.  Otherwise it will */
   /*          be excluded (NULL).                                      */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Read_Features_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, PLXS_Features_Data_t *Features);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Read_Features_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, PLXS_Features_Data_t *Features);
#endif

   /* The following function is responsible for sending an indication   */
   /* for a PLXS Spot-Check Measurement to a PLXS Client.  The first    */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the PLXS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Connection ID.  The final parameter is a    */
   /* pointer to the Spot-Check Measurement data to indicate to the PLXS*/
   /* Client.  This function returns a positive non-zero value if       */
   /* successful representing the GATT Transaction ID of the indication */
   /* or a negative error code if an error occurs.                      */
   /* * NOTE * It is the application's responsibilty to make sure that  */
   /*          the PLXS Spot-Check Measurement that is going to be      */
   /*          indicated has been previously configured for indications.*/
   /*          A PLXS Client MUST have written the PLXS Spot-Check      */
   /*          Measurement's Client Characteristic Configuration        */
   /*          Descriptor (CCCD) to enable indications.                 */
   /* * NOTE * This indication MUST be confirmed by the PLXS Client.    */
   /*          The PLXS Server will receive the                         */
   /*          etPLXS_Server_Confirmation event when the indication has */
   /*          been confirmed.                                          */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Indicate_Spot_Check_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, PLXS_Spot_Check_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Indicate_Spot_Check_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, PLXS_Spot_Check_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for sending a notification  */
   /* for a PLXS Continuous Measurement to a PLXS Client.  The first    */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the PLXS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Connection ID.  The final parameter is a    */
   /* pointer to the Continuous Measurement data to notify to the PLXS  */
   /* Client.  This function returns a positive non-zero value if       */
   /* successful representing the length of the notification or a       */
   /* negative error code if an error occurs.                           */
   /* * NOTE * It is the application's responsibilty to make sure that  */
   /*          the PLXS Continuous Measurement that is going to be      */
   /*          notified has been previously configured for              */
   /*          notifications.  A PLXS Client MUST have written the PLXS */
   /*          Continuous Measurement's Client Characteristic           */
   /*          Configuration Descriptor (CCCD) to enable notifications. */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Notify_Continuous_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, PLXS_Continuous_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Notify_Continuous_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, PLXS_Continuous_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for responding to a Record  */
   /* Access Control Point (RACP) request received from a PLXS Client.  */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to either of the PLXS_Initialize_XXX() functions. */
   /* The third parameter is the GATT Transaction ID of the request.    */
   /* The fourth parameter is the ErrorCode to indicate the type of     */
   /* response that will be sent.  This function returns a zero if      */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject Record Access Control Point (RACP) request when   */
   /*          the RACP has not been configured for indications, the    */
   /*          PLXS Client does not have proper security                */
   /*          (authentication, authorization, or encryption), or an    */
   /*          RACP procedure is already in progress.  All other reasons*/
   /*          should return PLXS_ERROR_CODE_SUCCESS for the ErrorCode. */
   /* * NOTE * This function does not indicate that the request was     */
   /*          successful, only that it has been accepted and is in     */
   /*          progress on the PLXS Server.  An indication MUST be sent */
   /*          if the RACP Request has been accepted to indicate the    */
   /*          result of the RACP Procedure.  The function              */
   /*          PLXS_Indicate_RACP_Response() MUST be used to indicate   */
   /*          this result.                                             */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          PLXS_ERROR_CODE_XXX from PLXSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI PLXS_RACP_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_RACP_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending an indication   */
   /* for the PLXS RACP Response to a PLXS Client.  The first parameter */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* either of the PLXS_Initialize_XXX() functions.  The third         */
   /* parameter is the GATT Connection ID.  The final parameter is a    */
   /* pointer to the RACP Response data to indicate to the PLXS Client. */
   /* This function returns a positive non-zero value if successful     */
   /* representing the GATT Transaction ID of the indication or a       */
   /* negative error code if an error occurs.                           */
   /* * NOTE * This function is used to send the RACP Procedure result  */
   /*          for the previously accepted RACP Request once the        */
   /*          procedure has completed.                                 */
   /* * NOTE * It is the application's responsibilty to make sure that  */
   /*          the PLXS RACP that is going to be indicated has been     */
   /*          previously configured for indications.  A PLXS Client    */
   /*          MUST have written the PLXS RACP's Client Characteristic  */
   /*          Configuration Descriptor (CCCD) to enable indications.   */
   /* * NOTE * This indication MUST be confirmed by the PLXS Client.    */
   /*          The PLXS Server will receive the                         */
   /*          etPLXS_Server_Confirmation event when the indication has */
   /*          been confirmed.                                          */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Indicate_RACP_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, PLXS_RACP_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Indicate_RACP_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, PLXS_RACP_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from a PLXS Client for a PLXS Characteristic's Client     */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the PLXS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is the ErrorCode to indicate the type of response that  */
   /* will be sent.  The fifth parameter is the CCCD type, which        */
   /* identifies the Characteristic, whose CCCD has been requested.  The*/
   /* final parameter contains the current Client Characteristic        */
   /* Configuration to send to the PLXS Client.  This function returns a*/
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          PLXS_ERROR_CODE_XXX from PLXSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The ClientConfiguration parameter is only REQUIRED if the*/
   /*          ErrorCode parameter is PLXS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it will be ignored.                            */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, PLXS_CCCD_Type_t Type, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, PLXS_CCCD_Type_t Type, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from a PLXS Client for a PLXS Characteristic's Client     */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the PLXS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is the ErrorCode to indicate the type of response that  */
   /* will be sent.  The final parameter is the CCCD type, which        */
   /* identifies the Characteristic, whose CCCD has been requested.     */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          PLXS_ERROR_CODE_XXX from PLXSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, PLXS_CCCD_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, PLXS_CCCD_Type_t Type);
#endif

   /* PLXS Client API.                                                  */

   /* The following function is responsible for parsing a value received*/
   /* in a GATT Read response, from the PLXS Server, interpreting it as */
   /* the PLXS Features.  The first parameter is the length of the Value*/
   /* received from the PLXS Server.  The second parameter is a pointer */
   /* to the Value received from the PLXS Server.  The final parameter  */
   /* is a pointer to store the parsed PLXS Features.  This function    */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Decode_Features(unsigned int ValueLength, Byte_t *Value, PLXS_Features_Data_t *Features);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Decode_Features_t)(unsigned int ValueLength, Byte_t *Value, PLXS_Features_Data_t *Features);
#endif

   /* The following function is responsible for parsing a value received*/
   /* in an indication, from the PLXS Server, interpreting it as a PLXS */
   /* Spot-Check Measurement.  The first parameter is the length of the */
   /* Value received from the PLXS Server.  The second parameter is a   */
   /* pointer to the Value received from the PLXS Server.  The final    */
   /* parameter is a pointer to store the parsed PLXS Spot-Check        */
   /* Measurement .  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * After decoding the PLXS Spot-Check Measurement, the PLXS */
   /*          Client MUST call GATT_Handle_Value_Confirmation() to     */
   /*          confirm that the indication has been received.           */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Decode_Spot_Check_Measurement(unsigned int ValueLength, Byte_t *Value, PLXS_Spot_Check_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Decode_Spot_Check_Measurement_t)(unsigned int ValueLength, Byte_t *Value, PLXS_Spot_Check_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* in a notification, from the PLXS Server, interpreting it as a PLXS*/
   /* Continuous Measurement.  The first parameter is the length of the */
   /* Value received from the PLXS Server.  The second parameter is a   */
   /* pointer to the Value received from the PLXS Server.  The final    */
   /* parameter is a pointer to store the parsed PLXS Continuous        */
   /* Measurement .  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Decode_Continuous_Measurement(unsigned int ValueLength, Byte_t *Value, PLXS_Continuous_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Decode_Continuous_Measurement_t)(unsigned int ValueLength, Byte_t *Value, PLXS_Continuous_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* in an indication, from the PLXS Server, interpreting it as a PLXS */
   /* Record Access Control Point (RACP) response.  The first parameter */
   /* is the length of the Value received from the PLXS Server.  The    */
   /* second parameter is a pointer to the Value received from the PLXS */
   /* Server.  The final parameter is a pointer to store the parsed PLXS*/
   /* RACP Response data.  This function returns a zero if successful or*/
   /* a negative return error code if an error occurs.                  */
   /* * NOTE * Before decoding the PLXS RACP Response, the PLXS Client  */
   /*          MUST call GATT_Handle_Value_Confirmation() to confirm    */
   /*          that the indication has been received.                   */
BTPSAPI_DECLARATION int BTPSAPI PLXS_Decode_RACP_Response(unsigned int ValueLength, Byte_t *Value, PLXS_RACP_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PLXS_Decode_RACP_Response_t)(unsigned int ValueLength, Byte_t *Value, PLXS_RACP_Response_Data_t *ResponseData);
#endif

#endif
