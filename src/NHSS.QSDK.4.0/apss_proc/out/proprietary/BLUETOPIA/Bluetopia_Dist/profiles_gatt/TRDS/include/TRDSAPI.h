/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< trdsapi.h >************************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TRDSAPI - Qualcomm Technologies Bluetooth Transport Discovery Service     */
/*           (GATT based) Type Definitions, Prototypes, and Constants.        */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/26/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __TRDSAPIH__
#define __TRDSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "TRDSTypes.h"     /* Transport Discovery Service Types/Constants     */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define TRDS_ERROR_INVALID_PARAMETER                     (-1000)
#define TRDS_ERROR_INVALID_BLUETOOTH_STACK_ID            (-1001)
#define TRDS_ERROR_INSUFFICIENT_RESOURCES                (-1002)
#define TRDS_ERROR_SERVICE_ALREADY_REGISTERED            (-1003)
#define TRDS_ERROR_INVALID_INSTANCE_ID                   (-1004)
#define TRDS_ERROR_MALFORMATTED_DATA                     (-1005)
#define TRDS_ERROR_INSUFFICIENT_BUFFER_SPACE             (-1006)
#define TRDS_ERROR_INVALID_CCCD_TYPE                     (-1007)
#define TRDS_ERROR_INVALID_ATTRIBUTE_HANDLE              (-1008)
#define TRDS_ERROR_CONTROL_POINT_NOT_SUPPORTED           (-1009)
#define TRDS_ERROR_MTU_EXCEEDED                          (-1010)

   /* The following structure defines the information needed to         */
   /* initialize the TDS Server.                                        */
   /* * NOTE * The Control_Point_Supported field indicates if the TDS   */
   /*          Server supports the TDS Control Point.  If this field is */
   /*          TRUE, then a Client Characteristic Configuration         */
   /*          Descriptor (CCCD) will automatically be included so each */
   /*          TDS Client may configure the TDS Control Point for       */
   /*          indications.                                             */
typedef struct _tagTRDS_Initialize_Data_t
{
   Boolean_t Control_Point_Supported;
} TRDS_Initialize_Data_t;

#define TRDS_INITIALIZE_DATA_SIZE                        (sizeof(TRDS_Initialize_Data_t))

   /* The following structure contains the handles that will need to be */
   /* cached by a TDS client in order to only do service discovery once.*/
typedef struct _tagTRDS_Client_Information_t
{
   Word_t Control_Point;
   Word_t Control_Point_CCCD;
} TRDS_Client_Information_t;

#define TRDS_CLIENT_INFORMATION_DATA_SIZE                (sizeof(TRDS_Client_Information_t))

   /* The following structure contains the Client Characteristic        */
   /* Configuration Descriptor (CCCD) Configuration that will need to be*/
   /* cached by a TDS Server for each TDS Client that connects to it.   */
typedef struct _tagTRDS_Server_Information_t
{
   Word_t Control_Point_Configuration;
} TRDS_Server_Information_t;

#define TRDS_SERVER_INFORMATION_DATA_SIZE                (sizeof(TRDS_Server_Information_t))

   /* The following enumeration defines the TDS Client Characteristic   */
   /* Configuration Descriptor (CCCD) types.                            */
typedef enum
{
   tctControlPoint
} TRDS_CCCD_Type_t;

   /* The following enumeration defines the TDS Roles.                  */
   /* * NOTE * The Transport Role MUST be set at bits 0 and 1 of the    */
   /*          Flags field in the TRDS_Transport_Block_Data_t structure.*/
typedef enum
{
   ttrNotSpecified,
   ttrSeekerOnly,
   ttrProviderOnly,
   ttrSeekerAndProvider
} TRDS_Transport_Role_t;

   /* The following enumeration defines the TDS Transport State.        */
   /* * NOTE * The Transport Role MUST be set at bits 3 and 4 of the    */
   /*          Flags field in the TRDS_Transport_Block_Data_t structure.*/
typedef enum
{
   ttsOff,
   ttsOn,
   ttsTemporarilyUnavailable,
   ttsRFU
} TRDS_Transport_State_t;

   /* The following structure defines the TDS Transport Block data.     */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the length of the TDS Transport Block data does not      */
   /*          exceed the maximum size of the advertising data.  It is  */
   /*          worth noting that multiple TDS Transport Blocks may be   */
   /*          included in the advertising data.                        */
   /* * NOTE * If a TDS Transport Block is incomplete (meaning it CANNOT*/
   /*          all fit in the advertising data), then bit 2 of the Flags*/
   /*          field MUST be set to indicate that the Transport Block is*/
   /*          incomplete.                                              */
typedef struct _tagTRDS_Transport_Block_Data_t
{
   Byte_t  Organization_ID;
   Byte_t  Flags;
   Byte_t  Transport_Data_Length;
   Byte_t *Transport_Data;
} TRDS_Transport_Block_Data_t;

#define TRDS_TRANSPORT_BLOCK_DATA_SIZE                   (sizeof(TRDS_Transport_Block_Data_t))

   /* The following enumeration defines the valid values that may be set*/
   /* for the Op_Code field of the TRDS_RACP_Request_Data_t structure.  */
typedef enum
{
   cprActivateTransport = TRDS_CONTROL_POINT_OP_CODE_ACTIVATE_TRANSPORT
} TRDS_Control_Point_Request_Type_t;

   /* The following structure defines the TDS Control Point Request data*/
   /* that may be sent in a TDS Control Point Request.                  */
   /* * NOTE * The maximum size of the Parameter_Length field is        */
   /*          dependent on the negotiated Maximum Transmission Unit    */
   /*          (MTU) size.  Two octets MUST be subtracted from the MTU  */
   /*          size for the Mandatory Op_Code and Organization_ID       */
   /*          fields.  The remaining length may be used for the        */
   /*          Parameter_Length field.                                  */
typedef struct _tagTRDS_Control_Point_Request_Data_t
{
   TRDS_Control_Point_Request_Type_t  Op_Code;
   Byte_t                            Organization_ID;
   Word_t                            Parameter_Length;
   Byte_t                           *Parameter;
} TRDS_Control_Point_Request_Data_t;

#define TRDS_CONTROL_POINT_REQUEST_DATA_SIZE             (sizeof(TRDS_Control_Point_Request_Data_t))

   /* The following enumeration defines the valid values that may be set*/
   /* for the Result_Code field of the                                  */
   /* TRDS_Control_Point_Response_Data_t structure.                     */
typedef enum
{
   tcpSuccess                   = TRDS_CONTROL_POINT_RESULT_CODE_SUCCESS,
   tcpOpCodeNotSupported        = TRDS_CONTROL_POINT_RESULT_CODE_OP_CODE_NOT_SUPPORTED,
   tcpInvalidParameter          = TRDS_CONTROL_POINT_RESULT_CODE_INVALID_PARAMETER,
   tcpUnsupportedOrganizationID = TRDS_CONTROL_POINT_RESULT_CODE_UNSUPPORTED_ORGANIZATION_ID,
   tcpOperationFailed           = TRDS_CONTROL_POINT_RESULT_CODE_OPERATION_FAILED
} TRDS_Control_Point_Result_Code_t;

   /* The following structure defines the TDS Control Point Response    */
   /* data that may be sent in a TDS Control Point Response.            */
   /* * NOTE * The maximum size of the Parameter_Length field is        */
   /*          dependent on the negotiated Maximum Transmission Unit    */
   /*          (MTU) size.  Two octets MUST be subtracted from the MTU  */
   /*          size for the Mandatory Request_Op_Code and Result_Code   */
   /*          fields.  The remaining length may be used for the        */
   /*          Parameter_Length field.                                  */
typedef struct _tagTRDS_Control_Point_Response_Data_t
{
   TRDS_Control_Point_Request_Type_t  Request_Op_Code;
   TRDS_Control_Point_Result_Code_t   Result_Code;
   Word_t                             Parameter_Length;
   Byte_t                            *Parameter;
} TRDS_Control_Point_Response_Data_t;

#define TRDS_CONTROL_POINT_RESPONSE_DATA_SIZE            (sizeof(TRDS_Control_Point_Response_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* TDS for the TDS Server.  These are used to determine the type of  */
   /* each event generated, and to ensure the proper union element is   */
   /* accessed for the TRDS_Event_Data_t structure.                     */
typedef enum _tagTRDS_Event_Type_t
{
   etTRDS_Server_Write_Control_Point_Request,
   etTRDS_Server_Read_CCCD_Request,
   etTRDS_Server_Write_CCCD_Request,
   etTRDS_Server_Confirmation
} TRDS_Event_Type_t;

   /* The following TDS Server Event is dispatched to a TDS Server when */
   /* a TDS Client has requested to write the TDS Control Point         */
   /* Characteristic.  The InstanceID identifies the TDS Instance that  */
   /* dispatched the event.  The ConnectionID identifies the GATT       */
   /* Connection Identifier for the request.  The ConnectionType        */
   /* identifies the GATT Connection type.  The TransactionID identifies*/
   /* the GATT Transaction Identifier for the request.  The RemoteDevice*/
   /* identifies the Bluetooth Address of the TDS Client.  The          */
   /* RequestData represents the RACP request data that has been        */
   /* received from the TDS Client.                                     */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the                                    */
   /*          TRDS_Write_Control_Point_Request_Response() function.    */
   /*          Some of the fields below are needed for the response.    */
   /*          This response does not indicate if the TDS Control Point */
   /*          request was successful.  It simply indicates that the    */
   /*          request has been accepted and is being processed.  Once  */
   /*          the TDS Control Point request has been processed an      */
   /*          indication MUST be sent for the result of the TDS Control*/
   /*          Point Procedure.                                         */
typedef struct _tagTRDS_Write_Control_Point_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   GATT_Connection_Type_t            ConnectionType;
   unsigned int                      TransactionID;
   BD_ADDR_t                         RemoteDevice;
   TRDS_Control_Point_Request_Data_t RequestData;
} TRDS_Write_Control_Point_Request_Data_t;

#define TRDS_WRITE_CONTROL_POINT_REQUEST_DATA_SIZE       (sizeof(TRDS_Write_Control_Point_Request_Data_t))

   /* The following TDS Server Event is dispatched to a TDS Server when */
   /* a TDS Client has requested to read a Client Characteristic        */
   /* Configuration descriptor (CCCD).  The InstanceID identifies the   */
   /* TDS Instance that dispatched the event.  The ConnectionID         */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* TDS Client.  The Type field is an enumeration that identifies the */
   /* CCCD type that has been requested.                                */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the TRDS_Read_CCCD_Request_Response()  */
   /*          function.  Some of the fields below are needed for the   */
   /*          response.                                                */
typedef struct _tagTRDS_Read_CCCD_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   TRDS_CCCD_Type_t       Type;
} TRDS_Read_CCCD_Request_Data_t;

#define TRDS_READ_CCCD_REQUEST_DATA_SIZE                 (sizeof(TRDS_Read_CCCD_Request_Data_t))

   /* The following TDS Server Event is dispatched to a TDS Server when */
   /* a TDS Client has requested to write a Client Characteristic       */
   /* Configuration descriptor (CCCD).  The InstanceID identifies the   */
   /* TDS Instance that dispatched the event.  The ConnectionID         */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* TDS Client.  The Type field is an enumeration that identifies the */
   /* CCCD type that has been requested.  The Configuration field is the*/
   /* value for the CCCD that has been requested to be written.         */
   /* * NOTE * If this request has been received, then it MUST be       */
   /*          responded to with the TRDS_Write_CCCD_Request_Response() */
   /*          function.  Some of the fields below are needed for the   */
   /*          response.                                                */
typedef struct _tagTRDS_Write_CCCD_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   TRDS_CCCD_Type_t       Type;
   Word_t                 Configuration;
} TRDS_Write_CCCD_Request_Data_t;

#define TRDS_WRITE_CCCD_REQUEST_DATA_SIZE                (sizeof(TRDS_Write_CCCD_Request_Data_t))

   /* The following is dispatched to a TDS Server when a TDS Client     */
   /* confirms an outstanding indication.  The InstanceID identifies the*/
   /* TDS Instance that dispatched the event.  The ConnectionID         */
   /* identifies the GATT Connection Identifier for the request.  The   */
   /* ConnectionType identifies the GATT Connection type.  The          */
   /* TransactionID identifies the GATT Transaction Identifier for the  */
   /* request.  The RemoteDevice identifies the Bluetooth Address of the*/
   /* TDS Client.  The Status field contains the result of the          */
   /* confirmation.  The BytesWritten field indicates the number of     */
   /* bytes that were successfully indicated to the TDS Client.         */
typedef struct _tagTRDS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
   Word_t                 BytesWritten;
} TRDS_Confirmation_Data_t;

#define TRDS_CONFIRMATION_DATA_SIZE                      (sizeof(TRDS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all TDS Service Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagTRDS_Event_Data_t
{
   TRDS_Event_Type_t Event_Data_Type;
   Word_t            Event_Data_Size;
   union
   {
      TRDS_Write_Control_Point_Request_Data_t *TRDS_Write_Control_Point_Request_Data;
      TRDS_Read_CCCD_Request_Data_t           *TRDS_Read_CCCD_Request_Data;
      TRDS_Write_CCCD_Request_Data_t          *TRDS_Write_CCCD_Request_Data;
      TRDS_Confirmation_Data_t                *TRDS_Confirmation_Data;
   } Event_Data;
} TRDS_Event_Data_t;

#define TRDS_EVENT_DATA_SIZE                             (sizeof(TRDS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a TDS Profile Event Receive Data Callback.  This function will be */
   /* called whenever an TDS Profile Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the TDS Event Data that        */
   /* occurred and the TDS Profile Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the TDS Profile Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer This function is guaranteed NOT to be invoked */
   /* more than once simultaneously for the specified installed callback*/
   /* (i.e.  this function DOES NOT have to be re-entrant).It needs to  */
   /* be noted however, that if the same Callback is installed more than*/
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another TDS Profile  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving TDS Profile Event   */
   /*            Packets.  A Deadlock WILL occur because NO TDS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *TRDS_Event_Callback_t)(unsigned int BluetoothStackID, TRDS_Event_Data_t *TRDS_Event_Data, unsigned long CallbackParameter);

   /* TDS Server API.                                                   */

   /* The following function is responsible for opening a TDS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the TDS Service Flags  */
   /* (TRDS_SERVICE_FLAGS_XXX) from TDSTypes.h.  These flags MUST be    */
   /* used to register the GATT service for the correct transport.  The */
   /* third parameter is a pointer to the TRDS_Initialize_Data_t        */
   /* structure that contains the information needed to initialize and  */
   /* configure the service.  The fourth parameter is the Callback      */
   /* function to call when an event occurs on this Server Port.  The   */
   /* fifth parameter is a user-defined callback parameter that will be */
   /* passed to the callback function with each event.  The final       */
   /* parameter is a pointer to store the GATT Service ID of the        */
   /* registered TDS service.  This function returns the positive,      */
   /* non-zero, Instance ID or a negative error code.                   */
   /* * NOTE * Only 1 TDS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, TRDS_Initialize_Data_t *InitializeData, TRDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, TRDS_Initialize_Data_t *InitializeData, TRDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a TDS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the TDS Service Flags  */
   /* (TRDS_SERVICE_FLAGS_XXX) from TDSTypes.h.  These flags MUST be    */
   /* used to register the GATT service for the correct transport.  The */
   /* third parameter is a pointer to the TRDS_Initialize_Data_t        */
   /* structure that contains the information needed to initialize and  */
   /* configure the service.  The fourth parameter is the Callback      */
   /* function to call when an event occurs on this Server Port.  The   */
   /* fifth parameter is a user-defined callback parameter that will be */
   /* passed to the callback function with each event.  The sixth       */
   /* parameter is a pointer to store the GATT Service ID of the        */
   /* registered TDS service.  The final parameter is a pointer, that on*/
   /* input can be used to control the location of the service in the   */
   /* GATT database, and on ouput to store the service handle range.    */
   /* This function returns the positive, non-zero, Instance ID or a    */
   /* negative error code.                                              */
   /* * NOTE * Only 1 TDS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, TRDS_Initialize_Data_t *InitializeData, TRDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, TRDS_Initialize_Data_t *InitializeData, TRDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previous TDS  */
   /* Server.  The first parameter is the Bluetooth Stack ID on which to*/
   /* close the server.  The second parameter is the InstanceID that was*/
   /* returned from a successful call to either of the                  */
   /* TRDS_Initialize_XXX() functions.  This function returns a zero if */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI TRDS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_TRDS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* TRDS_Suspend()).  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to TRDS_Suspend().  This    */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the TDS Service that is          */
   /* registered with a call to either of the TRDS_Initialize_XXX()     */
   /* functions.  The first parameter is the Bluetooth Stack ID of the  */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to either of the TRDS_Initialize_XXX()     */
   /* functions.  This function returns the non-zero number of          */
   /* attributes that are contained in a TDS Server or zero on failure. */
BTPSAPI_DECLARATION unsigned int BTPSAPI TRDS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_TRDS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for responding to a TDS     */
   /* Control Point request received from a TDS Client.  The first      */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the TRDS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is the ErrorCode to indicate the type of response that  */
   /* will be sent.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject the TDS Control Point request when the TDS Control*/
   /*          Point has not been configured for indications, the TDS   */
   /*          Client does not have proper security (authentication,    */
   /*          authorization, or encryption), or a TDS Control Point    */
   /*          procedure is already in progress.  All other reasons     */
   /*          should return TRDS_ERROR_CODE_SUCCESS for the ErrorCode. */
   /* * NOTE * This function does not indicate that the request was     */
   /*          successful, only that it has been accepted and is in     */
   /*          progress on the TDS Server.  An indication MUST be sent  */
   /*          if the TDS Control Point Request has been accepted to    */
   /*          indicate the result of the TDS Control Point Procedure.  */
   /*          The function TRDS_Indicate_Control_Point_Response() MUST */
   /*          be used to indicate this result.                         */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          TRDS_ERROR_CODE_XXX from TDSTypes.h or                   */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Write_Control_Point_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Write_Control_Point_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending an indication   */
   /* for the TDS Control Point Response to a TDS Client.  The first    */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the TRDS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Connection ID.  The final parameter is a    */
   /* pointer to the Control Point Response data to indicate to the TDS */
   /* Client.  This function returns a positive non-zero value if       */
   /* successful representing the GATT Transaction ID of the indication */
   /* or a negative error code if an error occurs.                      */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the negotiated maximum transmission unit (MTU-3) is large*/
   /*          enough to hold all fields of the Response Data Parameter.*/
   /*          This function will NOT allow the ResponseData parameter  */
   /*          to exceed the negotiated maximum transmission unit       */
   /*          (MTU-3).                                                 */
   /* * NOTE * This function is used to send the TDS Control Point      */
   /*          Procedure result once the TDS Control Point procedure has*/
   /*          completed for the previously accepted TDS Control Point  */
   /*          request.                                                 */
   /* * NOTE * It is the application's responsibilty to make sure that  */
   /*          the TDS Control Point that is going to be indicated has  */
   /*          been previously configured for indications.  A TDS Client*/
   /*          MUST have written the TDS Control Point's Client         */
   /*          Characteristic Configuration Descriptor (CCCD) to enable */
   /*          indications.                                             */
   /* * NOTE * This indication MUST be confirmed by the TDS Client.  The*/
   /*          TDS Server will receive the etTRDS_Server_Confirmation   */
   /*          event when the indication has been confirmed.            */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Indicate_Control_Point_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, TRDS_Control_Point_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Indicate_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, TRDS_Control_Point_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from a TDS Client for a TDS Characteristic's Client       */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the TRDS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is the ErrorCode to indicate the type of response that  */
   /* will be sent.  The fifth parameter is the CCCD type, which        */
   /* identifies the Characteristic, whose CCCD has been requested.  The*/
   /* final parameter contains the current Client Characteristic        */
   /* Configuration to send to the TDS Client.  This function returns a */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          TRDS_ERROR_CODE_XXX from TDSTypes.h or                   */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The ClientConfiguration parameter is only REQUIRED if the*/
   /*          ErrorCode parameter is TRDS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it will be ignored.                            */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, TRDS_CCCD_Type_t Type, Word_t Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, TRDS_CCCD_Type_t Type, Word_t Configuration);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from a TDS Client for a TDS Characteristic's Client       */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the TRDS_Initialize_XXX() functions.  The third      */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is the ErrorCode to indicate the type of response that  */
   /* will be sent.  The final parameter is the CCCD type, which        */
   /* identifies the Characteristic, whose CCCD has been requested.     */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          TRDS_ERROR_CODE_XXX from TDSTypes.h or                   */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, TRDS_CCCD_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, TRDS_CCCD_Type_t Type);
#endif

   /* TDS Client API.                                                   */

   /* The following function is responsible for formatting the TDS      */
   /* Control Point Request into a user-specified buffer, for a GATT    */
   /* Write request, that will be sent to the TDS Server.  This function*/
   /* may also be used to determine the minimum size of the buffer      */
   /* needed to hold the formatted data (see below).  The first         */
   /* parameter is the TDS Control Point Request data that will be      */
   /* formatted into the user-specified buffer.  The second parameter is*/
   /* the length of the user-specified Buffer.  The final parameter is a*/
   /* pointer to the user-specified Buffer that will hold the formatted */
   /* data if this function is successful.  This function returns zero  */
   /* if the TDS Control Point Request data has been successfully       */
   /* formatted into the user-specified buffer.  If this function is    */
   /* used to determine the size of the buffer to hold the formatted    */
   /* data, then a positive non-zero value will be returned.  Otherwise */
   /* this function will return a negative error code if an error       */
   /* occurs.                                                           */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the length of the formatted user-specified buffer to send*/
   /*          in the GATT Write Request does NOT exceed the negotiatied*/
   /*          maximum transmission unit (MTU-3).  Otherwise, it cannot */
   /*          fit in a GATT_Write_Request().  The application may call */
   /*          GATT_Query_Connection_MTU() to determine the MTU size for*/
   /*          the GATT Connection.                                     */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The TDS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Format_Control_Point_Request(TRDS_Control_Point_Request_Data_t *RequestData, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Format_Control_Point_Request_t)(TRDS_Control_Point_Request_Data_t *RequestData, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* in an indication, from the TDS Server, interpreting it as a TDS   */
   /* Control Point response.  The first parameter is the length of the */
   /* Value received from the TDS Server.  The second parameter is a    */
   /* pointer to the Value received from the TDS Server.  The final     */
   /* parameter is a pointer to store the parsed TDS Control Point      */
   /* Response data.  This function returns a zero if successful or a   */
   /* negative return error code if an error occurs.                    */
   /* ** NOTE ** If the Parameter_Length field of the                   */
   /*            TRDS_Control_Point_Response_Data_t struture is non-zero*/
   /*            on successful return from this function, then a        */
   /*            response parameter has been received.  The Parameter   */
   /*            field of the TRDS_Control_Point_Response_Data_t        */
   /*            structure MUST be copied on successful return from this*/
   /*            function if the application needs to store the value.  */
   /*            This pointer is ONLY VALID, while the indicate event   */
   /*            received via callback is valid.                        */
   /* * NOTE * Before decoding the TDS Control Point Response, the TDS  */
   /*          Client MUST call GATT_Handle_Value_Confirmation() to     */
   /*          confirm that the indication has been received.           */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Decode_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, TRDS_Control_Point_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Decode_Control_Point_Response_t)(unsigned int ValueLength, Byte_t *Value, TRDS_Control_Point_Response_Data_t *ResponseData);
#endif

   /* TDS Server and Client API.                                        */

   /* The following function is responsible for formatting the TDS      */
   /* Transport Block into a user-specified buffer.  This function may  */
   /* also be used to determine the minimum size of the buffer to hold  */
   /* the formatted data (see below).  The first parameter is the TDS   */
   /* Transport Block data that will be formatted into the              */
   /* user-specified buffer.  The second parameter is the length of the */
   /* user-specified Buffer.  The final parameter is a pointer to the   */
   /* user-specified Buffer that will hold the formatted data if this   */
   /* function is successful.  This function returns a positive non-zero*/
   /* value if successful (see below about the return value) or a       */
   /* negative error code if an error occurs.                           */
   /* * NOTE * It is the application's responsibility to ensure that the*/
   /*          TransportBlockData parameter is valid.                   */
   /* * NOTE * It is the application's responsibility to ensure that the*/
   /*          maximum size of the advertising data is not exceeded.    */
   /* * NOTE * This function may be called repeatedly to include        */
   /*          multiple TDS Transport Blocks in the advertising data.   */
   /* * NOTE * The Buffer and BufferLength parameters may be set to the */
   /*          location in the advertising data for the TDS Transport   */
   /*          Block and the remaining length of the advertising data.  */
   /* * NOTE * If the BufferLength parameter is positive non-zero, the  */
   /*          Buffer parameter MUST be valid, and this function will   */
   /*          return a positive non-zero value, which represents length*/
   /*          of the TDS Transport Data that has been formatted into   */
   /*          the Buffer.  The caller may use this size to determine   */
   /*          the location in the advertising data to include another  */
   /*          TDS Transport Block if the user specified Buffer points  */
   /*          to the advertising data.                                 */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the minimum    */
   /*          size of the buffer needed to hold the formatted data.    */
   /*          The caller may use this size to allocate a buffer        */
   /*          necessary to hold the formatted data.                    */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Format_Transport_Block(TRDS_Transport_Block_Data_t *TransportBlockData, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Format_Transport_Block_t)(TRDS_Transport_Block_Data_t *TransportBlockData, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a subset of     */
   /* advertising data, interpreting it as a TDS Transport Block.  The  */
   /* first parameter is the remaining length of the Transport Block    */
   /* Section.  The second parameter is a pointer to the location in the*/
   /* advertising data that follows the Transport Discovery Data AD Type*/
   /* Code.  The final parameter is a pointer to store the parsed TDS   */
   /* Control Point Response data.  This function returns a positive    */
   /* non-zero if successful, which represents the size of the decoded  */
   /* TDS Transport Block or a negative return error code if an error   */
   /* occurs.                                                           */
   /* ** NOTE ** If the Transport_Data_Length field of the              */
   /*            TRDS_Transport_Block_Data_t struture is non-zero on    */
   /*            successful return from this function, then the         */
   /*            Transport Data has been received in the TDS Transport  */
   /*            Block.  The Transport_Data field MUST be copied on     */
   /*            successful return from this function if the application*/
   /*            needs to store the value.  This pointer is ONLY VALID, */
   /*            while the Buffer parameter is Valid since it points to */
   /*            the location of the Transport Data.                    */
   /* * NOTE * It is the application's responsibility to verify the     */
   /*          decoded TransportBlockData parameter on successful return*/
   /*          from this function.                                      */
   /* * NOTE * This function may be called repeatedly to decode multiple*/
   /*          TDS Transport Blocks.  The TDS Transport Blocks are      */
   /*          located one after the other, and the positive return     */
   /*          value for success may be used to update the remaining    */
   /*          length and location in the advertising data for the next */
   /*          TDS Transport Block to decode.                           */
BTPSAPI_DECLARATION int BTPSAPI TRDS_Decode_Transport_Block(unsigned int RemainingLength, Byte_t *Buffer, TRDS_Transport_Block_Data_t *TransportBlockData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TRDS_Decode_Transport_Block_t)(unsigned int RemainingLength, Byte_t *Buffer, TRDS_Transport_Block_Data_t *TransportBlockData);
#endif

#endif
