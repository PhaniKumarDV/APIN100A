/*****< gatmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GATMAPI - Local Generic Attribute Profile Manager API for Stonestreet One */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/16/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GATMAPIH__
#define __GATMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "GATMMSG.h"             /* BTPM GATT Manager Message Formats.        */

   /* The following enumerated type represents the Generic Attribute    */
   /* Profile Manager Event Types that are dispatched by this module.   */
typedef enum
{
   /* GATM Connection Events.                                           */
   getGATTConnected,
   getGATTDisconnected,
   getGATTConnectionMTUUpdate,
   getGATTHandleValueData,

   /* GATM Client Events.                                               */
   getGATTReadResponse,
   getGATTWriteResponse,
   getGATTErrorResponse,

   /* GATM Server Events.                                               */
   getGATTWriteRequest,
   getGATTSignedWrite,
   getGATTReadRequest,
   getGATTPrepareWriteRequest,
   getGATTCommitPrepareWrite,
   getGATTHandleValueConfirmation
} GATM_Event_Type_t;

   /* GATM Connection Events.                                           */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getConnected event.  The        */
   /* ConnectionType parameters specifies the type of GATT connection.  */
   /* The RemoteDevice member specify the remote device that has        */
   /* connected.  The MTU member specifies the Maximum Transmission Unit*/
   /* of the connection.                                                */
typedef struct _tagGATM_Connected_Event_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Word_t                 MTU;
} GATM_Connected_Event_Data_t;

#define GATM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(GATM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getDisconnected event.  This    */
   /* event is dispatched when a remote device disconnects from the     */
   /* local device.  The ConnectionType parameters specifies the type of*/
   /* GATT connection that was just disconnected.  The                  */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the device that disconnected from the profile.                 */
typedef struct _tagGATM_Disconnected_Event_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} GATM_Disconnected_Event_Data_t;

#define GATM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(GATM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getGATTConnectionMTUUpdate      */
   /* event.  This event is dispatched when the MTU for a GATT          */
   /* connection is updated.  The ConnectionType parameters specifies   */
   /* the type of GATT connection.  The RemoteDeviceAddress member      */
   /* specifies the Bluetooth device address of the device that has the */
   /* updated MTU and the MTU member specifies the new MTU.             */
typedef struct _tagGATM_Connection_MTU_Update_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Word_t                 MTU;
} GATM_Connection_MTU_Update_Data_t;

#define GATM_CONNECTION_MTU_UPDATE_DATA_SIZE                   (sizeof(GATM_Connection_MTU_Update_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getHandleValueData event.  This */
   /* event is dispatched when a remote device sends either a GATT      */
   /* Handle Value Notification or Indication to the local device.  The */
   /* ConnectionType parameters specifies the type of GATT connection   */
   /* that send the Handle Value Indication/Notification.  The          */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the device that sent the GATT Handle Value                     */
   /* Indication/Notification.  The HandleValueIndication member is TRUE*/
   /* if the event is for an Indication and FALSE if the event is for a */
   /* Notification.  The AttributeHandle member identifies the attribute*/
   /* that is being indicated/notified.  The AttributeValueLength and   */
   /* AttributeValue members specify the length and the data that was   */
   /* contained in the Handle Value Indication/Notification.            */
typedef struct _tagGATM_Handle_Value_Data_Event_Data_t
{
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   Boolean_t               HandleValueIndication;
   Word_t                  AttributeHandle;
   Word_t                  AttributeValueLength;
   Byte_t                 *AttributeValue;
} GATM_Handle_Value_Data_Event_Data_t;

#define GATM_HANDLE_VALUE_DATA_EVENT_DATA_SIZE                 (sizeof(GATM_Handle_Value_Data_Event_Data_t))

   /* GATM Client Events.                                               */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a gctReadResponse event.  This    */
   /* event is dispatched when a previously started Read operation is   */
   /* completed sucessfully.  The ConnectionType parameters specifies   */
   /* the type of GATT connection that sent the Read Response.  The     */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the device that the read operation has completed succesfully   */
   /* for.  The TransactionID member contains the TransactionID that was*/
   /* returned in the successful call to GATM_ReadValue().  The Handle  */
   /* parameter contains the Handle of the attribute that was read.  The*/
   /* Final if TRUE specifies that the entire attribute has been        */
   /* written.  If FALSE this specifies that the value MAY contain more */
   /* data than was read.  The ValueLength and Value members contain the*/
   /* length and the data that was read from the remote device.         */
   /* * NOTE * This event is only dispatched to the registered callback */
   /*          specified in the call to the GATM_ReadValue() API.       */
typedef struct _tagGATM_Read_Response_Event_Data_t
{
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            TransactionID;
   Word_t                  Handle;
   Boolean_t               Final;
   unsigned int            ValueLength;
   Byte_t                 *Value;
} GATM_Read_Response_Event_Data_t;

#define GATM_READ_RESPONSE_EVENT_DATA_SIZE                     (sizeof(GATM_Read_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a gctWriteResponse event.  This   */
   /* event is dispatched when a previously started Write operation is  */
   /* completed sucessfully.  The ConnectionType parameters specifies   */
   /* the type of GATT connection that sent the Write Response.  The    */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the device that the write operation has completed for.  The    */
   /* TransactionID member contains the TransactionID that was returned */
   /* in the successful call to GATM_WriteValue().  The Handle          */
   /* specifies the Handle of the attribute that was succesfully        */
   /* written.                                                          */
   /* * NOTE * This event is only dispatched to the registered callback */
   /*          specified in the call to the GATM_WriteValue() API.      */
typedef struct _tagGATM_Write_Response_Event_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           TransactionID;
   Word_t                 Handle;
} GATM_Write_Response_Event_Data_t;

#define GATM_WRITE_RESPONSE_EVENT_DATA_SIZE                    (sizeof(GATM_Write_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a gctErrorResponse event.  This   */
   /* event is dispatched when an error response is sent to a previously*/
   /* started Read or Write operation.  The ConnectionType parameters   */
   /* specifies the type of GATT connection that sent the Error         */
   /* Response.  The RemoteDeviceAddress member specifies the Bluetooth */
   /* device address of the device that the error response has been     */
   /* received for.  The TransactionID member contains the TransactionID*/
   /* that was returned in the successful call to either                */
   /* GATM_ReadValue() or GATM_WriteValue().  The Handle specifies the  */
   /* handle of the attribute whose access (either a read or write)     */
   /* caused the error.  The ErrorType member specifies the type of     */
   /* error that occurred.  The AttributeProtocolErrorCode member (which*/
   /* is only valid if ErrorType is set to retErrorResponse) specifies  */
   /* the Attribute Protocol Error Code that was received in the error  */
   /* response from the remote device.                                  */
   /* * NOTE * This event is only dispatched to the registered callback */
   /*          specified in the call to the GATM_ReadValue() or         */
   /*          GATM_WriteValue() API whose operation caused the error   */
   /*          response.                                                */
typedef struct _tagGATM_Error_Response_Event_Data_t
{
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDeviceAddress;
   unsigned int              TransactionID;
   Word_t                    Handle;
   GATT_Request_Error_Type_t ErrorType;
   Byte_t                    AttributeProtocolErrorCode;
} GATM_Error_Response_Event_Data_t;

#define GATM_ERROR_RESPONSE_EVENT_DATA_SIZE                    (sizeof(GATM_Error_Response_Event_Data_t))

   /* GATM Server Events.                                               */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getWriteRequest event.  This    */
   /* event is dispatched when a remote device attempts to write a value*/
   /* to an attribute contained in a published service.  The            */
   /* ConnectionType parameters specifies the type of GATT connection   */
   /* that sent the Write Request.  The RemoteDeviceAddress member      */
   /* specifies the Bluetooth device address of the device that is      */
   /* making the write request.  The ServiceID specifies the ServiceID  */
   /* of the service whose attribute is being written to.  The          */
   /* RequestID, if non-zero, specifies that a response to the write is */
   /* requested.  The AttributeOffset specifies the offset of the       */
   /* Characteristic Value or Descriptor that is being written.  The    */
   /* final two parameters specify the length and a pointer to the data */
   /* that is being written.                                            */
   /* * NOTE * If RequestID is Non-ZERO then the write request must be  */
   /*          responded to either the GATM_WriteResponse() or          */
   /*          GATM_ErrorResponse() APIs.                               */
typedef struct _tagGATM_Write_Request_Event_Data_t
{
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            ServiceID;
   unsigned int            RequestID;
   unsigned int            AttributeOffset;
   unsigned int            DataLength;
   Byte_t                 *Data;
} GATM_Write_Request_Event_Data_t;

#define GATM_WRITE_REQUEST_EVENT_DATA_SIZE                     (sizeof(GATM_Write_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getSignedWrite event.  This     */
   /* event is dispatched when a remote device attempts to do a signed  */
   /* write a value to an attribute contained in a published service.   */
   /* The ConnectionType parameters specifies the type of GATT          */
   /* connection that sent the Signed Write.  The RemoteDeviceAddress   */
   /* member specifies the Bluetooth device address of the device that  */
   /* is making the write request.  The ServiceID specifies the         */
   /* ServiceID of the service whose attribute is being written to.  The*/
   /* AttributeOffset specifies the offset of the Characteristic Value  */
   /* or Descriptor that is being written.  The final two parameters    */
   /* specify the length and a pointer to the data that is being        */
   /* written.                                                          */
   /* * NOTE * The ValidSignature member specifies if the signed write  */
   /*          is valid.  If ValidSignature is TRUE then the signature  */
   /*          on this write command was verified.  If ValidSignature is*/
   /*          FALSE then the signature received from the remote device */
   /*          was not verified and this write shall be ignored.  More  */
   /*          action can be taken (i.e.  disconnect the device) at the */
   /*          applications discretion.                                 */
typedef struct _tagGATM_Signed_Write_Event_Data_t
{
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            ServiceID;
   Boolean_t               ValidSignature;
   unsigned int            AttributeOffset;
   unsigned int            DataLength;
   Byte_t                 *Data;
} GATM_Signed_Write_Event_Data_t;

#define GATM_SIGNED_WRITE_EVENT_DATA_SIZE                      (sizeof(GATM_Signed_Write_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getReadRequest event.  This     */
   /* event is dispatched when a remote device attempts to write a value*/
   /* to an attribute contained in a published service.  The            */
   /* ConnectionType parameters specifies the type of GATT connection   */
   /* that sent the Read Request.  The RemoteDeviceAddress member       */
   /* specifies the Bluetooth device address of the device that is      */
   /* making the read request.  The ServiceID specifies the ServiceID of*/
   /* the service whose attribute is being read.  The RequestID         */
   /* specifies the ID that can be used to respond to the read request. */
   /* The AttributeOffset specifies the offset of the Characteristic    */
   /* Value or Descriptor that is being read.  The final parameter      */
   /* specify the offset, in bytes, into the value that the client is   */
   /* attempting to read.                                               */
   /* * NOTE * Either the GATM_ReadResponse() or GATM_ErrorResponse()   */
   /*          APIs must be used to respond to this event.              */
typedef struct _tagGATM_Read_Request_Event_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   unsigned int           RequestID;
   unsigned int           AttributeOffset;
   unsigned int           AttributeValueOffset;
} GATM_Read_Request_Event_Data_t;

#define GATM_READ_REQUEST_EVENT_DATA_SIZE                      (sizeof(GATM_Read_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getPreprareWriteRequest event.  */
   /* This event is dispatched when a remote device attempts to write a */
   /* value to an attribute contained in a published service.  The      */
   /* ConnectionType parameters specifies the type of GATT connection   */
   /* that sent the Prepare Write Request.  The RemoteDeviceAddress     */
   /* member specifies the Bluetooth device address of the device that  */
   /* is making the prepare write request.  The ServiceID specifies the */
   /* ServiceID of the service whose attribute is being written to.  The*/
   /* RequestID specifies the RequestID that can be used to respond to  */
   /* the request.  The AttributeOffset specifies the offset of the     */
   /* Characteristic Value or Descriptor that is being written.  The    */
   /* AttributeValueOffset specifies the offset into the value that the */
   /* request is being made to.  The final two parameters specify the   */
   /* length and a pointer to the data that is being written.           */
   /* * NOTE * It is the responsibility of the application to wait for  */
   /*          the getCommitPrepareWrite event with a successful status */
   /*          before writing the attribute to the value.               */
   /* * NOTE * Either the GATM_WriteResponse() or GATM_ErrorResponse()  */
   /*          APIs must be used to respond to this request.            */
typedef struct _tagGATM_Prepare_Write_Request_Event_Data_t
{
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            ServiceID;
   unsigned int            RequestID;
   unsigned int            AttributeOffset;
   unsigned int            AttributeValueOffset;
   unsigned int            DataLength;
   Byte_t                 *Data;
} GATM_Prepare_Write_Request_Event_Data_t;

#define GATM_PREPARE_WRITE_REQUEST_EVENT_DATA_SIZE             (sizeof(GATM_Prepare_Write_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getCommitPrepareWrite event.    */
   /* This event is dispatched when a remote device attempts to commite */
   /* a write to a value to an attribute contained in a published       */
   /* service.  The ConnectionType parameters specifies the type of GATT*/
   /* connection that sent the Commit Prepare Write Event.  The         */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the device that is making the commit.  The ServiceID specifies */
   /* the ServiceID of the service whose should either commit writes or */
   /* de-commite writes for the specified client.  The final parameter  */
   /* specifies if the writes for the specified client should be        */
   /* commited (TRUE) or not committed (FALSE).                         */
   /* * NOTE * If the CommitWrites member is FALSE the application must */
   /*          NOT commit any queued prepare writes for the specified   */
   /*          client.                                                  */
typedef struct _tagGATM_Commit_Prepare_Write_Event_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   Boolean_t              CommitWrites;
} GATM_Commit_Prepare_Write_Event_Data_t;

#define GATM_COMMIT_PREPARE_WRITE_EVENT_DATA_SIZE             (sizeof(GATM_Commit_Prepare_Write_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getHandleValueConfirmation      */
   /* event.  This event is dispatched when a remote device sends a     */
   /* Handle Value Confirmation to a Handle Value Indication sent by the*/
   /* local device.  The ConnectionType parameters specifies the type of*/
   /* GATT connection that sent the Handle Value Confirmation.  The     */
   /* RemoteDeviceAddress member specifies the Bluetooth device address */
   /* of the device that sent the Handle Value Confirmation.  The       */
   /* ServiceID member specifies the ServiceID of the service whose     */
   /* attribute was indicated.  The TransactionID member - contains the */
   /* Transaction Identifier that was returned in the                   */
   /* GATM_SendHandleValueIndication() that sent the indication that    */
   /* caused this event to be generated.  The AttributeOffset member    */
   /* specifies the offset of the attribute that was indicated.  The    */
   /* Status member indicates the status of the confirmation.           */
typedef struct _tagGATM_Handle_Value_Confirmation_Event_Data_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   unsigned int           TransactionID;
   unsigned int           AttributeOffset;
   unsigned int           Status;
} GATM_Handle_Value_Confirmation_Event_Data_t;

#define GATM_HANDLE_VALUE_CONFIRMATION_EVENT_DATA_SIZE         (sizeof(GATM_Handle_Value_Confirmation_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Generic Attribute Profile Manager Connection Event (and Event     */
   /* Data) of a Generic Attribute Profile Manager Connection Event.    */
typedef struct _tagGATM_Event_Data_t
{
   GATM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      /* GATM Connection Events.                                        */
      GATM_Connected_Event_Data_t                 ConnectedEventData;
      GATM_Disconnected_Event_Data_t              DisconnectedEventData;
      GATM_Connection_MTU_Update_Data_t           ConnectionMTUUpdateEventData;
      GATM_Handle_Value_Data_Event_Data_t         HandleValueDataEventData;

      /* GATM Client Events.                                            */
      GATM_Read_Response_Event_Data_t             ReadResponseEventData;
      GATM_Write_Response_Event_Data_t            WriteResponseEventData;
      GATM_Error_Response_Event_Data_t            ErrorResponseEventData;

      /* GATM Server Events.                                            */
      GATM_Write_Request_Event_Data_t             WriteRequestData;
      GATM_Signed_Write_Event_Data_t              SignedWriteData;
      GATM_Read_Request_Event_Data_t              ReadRequestData;
      GATM_Prepare_Write_Request_Event_Data_t     PrepareWriteRequestEventData;
      GATM_Commit_Prepare_Write_Event_Data_t      CommitPrepareWriteEventData;
      GATM_Handle_Value_Confirmation_Event_Data_t HandleValueConfirmationEventData;
   } EventData;
} GATM_Event_Data_t;

#define GATM_EVENT_DATA_SIZE                                   (sizeof(GATM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a GATT Event Callback.  This function will be called whenever the */
   /* Generic Attribute Profile Manager dispatches a event (and the     */
   /* client has registered for events).  This function passes to the   */
   /* caller the Generic Attribute Profile Manager Event and the        */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the Event   */
   /* Data ONLY in the context of this callback.  If the caller requires*/
   /* the Data for a longer period of time, then the callback function  */
   /* MUST copy the data into another Data Buffer.  This function is    */
   /* guaranteed NOT to be invoked more than once simultaneously for the*/
   /* specified installed callback (i.e.  this function DOES NOT have be*/
   /* reentrant).  Because of this, the processing in this function     */
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another Message will not be processed while this function call is */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *GATM_Event_Callback_t)(GATM_Event_Data_t *EventData, void *CallbackParameter);

   /* GATM Connection APIs.                                             */

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a Local Generic Attribute Profile Profile     */
   /* Manager event callback function.  This callback will be called    */
   /* whenever a GATT event occurs in the system.  This function accepts*/
   /* the GATT Event Callback and Callback parameter (to receive GATT   */
   /* event) to register.  This function returns a positive, non-zero,  */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * A successful return value represents the Event Callback  */
   /*          ID that can be used to un-register the callback.         */
BTPSAPI_DECLARATION int BTPSAPI GATM_RegisterEventCallback(GATM_Event_Callback_t EventCallback, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_RegisterEventCallback_t)(GATM_Event_Callback_t EventCallback, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered GATM Event         */
   /* Callback.  The parameter to this function represents the positive */
   /* non-zero Callback ID that was returned from a succesful call to   */
   /* the GATM_RegisterEventCallback().  This function returns zero if  */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI GATM_UnRegisterEventCallback(unsigned int EventCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_UnRegisterEventCallback_t)(unsigned int EventCallbackID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the currently active GATT Connections.  This */
   /* function accepts a pointer to a buffer that will receive any      */
   /* active GATT Connections.  The first parameter specifies the       */
   /* maximum number of GATM_Connection_Information_t entries that the  */
   /* buffer will support (i.e.  can be copied into the buffer).  The   */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of GATT connections.  The final parameter   */
   /* can be used to retrieve the total number of GATT connections      */
   /* (regardless of the size of the list specified by the first two    */
   /* parameters).  This function returns a non-negative value if       */
   /* successful which represents the number of GATT connections that   */
   /* were copied into the specified input buffer.  This function       */
   /* returns a negative return error code if there was an error.       */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Connected List Entries, in which case the final parameter*/
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI GATM_QueryConnectedDevices(unsigned int MaximumConnectionListEntries, GATM_Connection_Information_t *ConnectionList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_QueryConnectedDevices_t)(unsigned int MaximumConnectionListEntries, GATM_Connection_Information_t *ConnectionList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* GATM Client APIs.                                                 */

   /* The following function is provided to allow a mechanism for local */
   /* modules to read a specified Attribute on the specified remote     */
   /* device.  The function accepts the EventCallbackID of the callback */
   /* that is to be notified of the result of the read, the BD_ADDR of  */
   /* the Remote Device to read the attribute from, the Attribute Handle*/
   /* of the attribute on the remote device to read, the number of bytes*/
   /* into the attribute to read and a boolean that if TRUE specifies   */
   /* that the GATT Manager should read the entire value (even if it    */
   /* takes multiple GATT packets to accomplish this).  This function   */
   /* returns a positive non-zero value if successful, or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
   /* * NOTE * The Offset parameter can be used to read the specified   */
   /*          attribute at the offset specified by the parameter.      */
   /* * NOTE * The ReadAll parameter specifies whether the entire       */
   /*          attribute should be read.  If TRUE the GATT Manager will */
   /*          read the entire specified Attribute.  If FALSE the GATT  */
   /*          Manager will only read the part of the attribute that    */
   /*          will fit in one packet.                                  */
BTPSAPI_DECLARATION int BTPSAPI GATM_ReadValue(unsigned int EventCallbackID, BD_ADDR_t RemoteDevice, Word_t AttributeHandle, unsigned int Offset, Boolean_t ReadAll);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_ReadValue_t)(unsigned int EventCallbackID, BD_ADDR_t RemoteDevice, Word_t AttributeHandle, unsigned int Offset, Boolean_t ReadAll);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to write a specified Attribute on the specified remote    */
   /* device.  The function accepts the EventCallbackID of the callback */
   /* that is to be notified of the result of the write, the BD_ADDR of */
   /* the Remote Device to write the value to, the Attribute Handle of  */
   /* the attribute on the remote device to write, the length of the    */
   /* data and a pointer to the data to write.  This function returns a */
   /* positive non-zero value if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
BTPSAPI_DECLARATION int BTPSAPI GATM_WriteValue(unsigned int EventCallbackID, BD_ADDR_t RemoteDevice, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_WriteValue_t)(unsigned int EventCallbackID, BD_ADDR_t RemoteDevice, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to perform a write without response to a specified        */
   /* Attribute on the specified remote device.  The first parameter is */
   /* the Callback ID of the previously registered callback.  The second*/
   /* parameter is the BD_ADDR of the connected device to write the     */
   /* value to.  The third parameter specifies the handle of the        */
   /* attribute that is to be written.  The fourth parameter specifies  */
   /* (if TRUE) that a signed write is to be performed to the attribute.*/
   /* The final two parameters specify the length and a pointer to the  */
   /* data that is to be written.  This function returns the number of  */
   /* bytes written on success, or a negative return error code if there*/
   /* was an error.                                                     */
   /* * NOTE * No event is generated by this function.                  */
   /* * NOTE * A Signed Write can only be performed to a previously     */
   /*          paired device.                                           */
BTPSAPI_DECLARATION int BTPSAPI GATM_WriteValueWithoutResponse(unsigned int EventCallbackID, BD_ADDR_t RemoteDevice, Word_t AttributeHandle, Boolean_t PerformedSignedWrite, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_WriteValueWithoutResponse_t)(unsigned int EventCallbackID, BD_ADDR_t RemoteDevice, Word_t AttributeHandle, Boolean_t PerformedSignedWrite, unsigned int DataLength, Byte_t *Data);
#endif

   /* GATM Server APIs.                                                 */

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a persistent UID (unique ID) for GATM Service */
   /* Registration.  The first parameter to this function is the number */
   /* of attributes that will be registered in the call to              */
   /* GATM_RegisterService().  The second parameter is a pointer to     */
   /* return the newly registered UID.  The final parameter is an       */
   /* optional pointer to store the handle range in the GATT database   */
   /* that the Persistent UID is allocated to (that is the range in the */
   /* GATT database that the persistent service will reside when it has */
   /* been published).  This function returns ZERO on success (in which */
   /* case PersistentUIDResult will contain a valid persistent UID) or a*/
   /* negative error code.                                              */
   /* * NOTE * The purpose of a persistent UID is to ensure that a      */
   /*          service is always registered at the same location in the */
   /*          local GATT database.  As long as the UID is registered   */
   /*          the application that registers it is ensured that no     */
   /*          other service will use it's allocated handle range       */
   /*          regardless of whether or not the service that is located */
   /*          at the handle range is published.                        */
   /* * NOTE * The NumberOfAttributes parameter MUST match the          */
   /*          NumberOfAttributes parameter passed to                   */
   /*          GATM_RegisterService() otherwise GATM_RegisterService()  */
   /*          will return an error code.                               */
BTPSAPI_DECLARATION int BTPSAPI GATM_RegisterPersistentUID(unsigned int NumberOfAttributes, DWord_t *PersistentUIDResult, GATT_Attribute_Handle_Group_t *ServiceHandleRangeResult);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_RegisterPersistentUID_t)(unsigned int NumberOfAttributes, DWord_t *PersistentUIDResult, GATT_Attribute_Handle_Group_t *ServiceHandleRangeResult);
#endif

   /* The following function is a utility function which is used to     */
   /* unregistered the specified persistent UID.  The only parameter to */
   /* this function is the persistent UID that was registered with the  */
   /* successful call to GATM_RegisterPersistentUID().  This function   */
   /* returns ZERO on success or a negative error code.                 */
BTPSAPI_DECLARATION int BTPSAPI GATM_UnRegisterPersistentUID(DWord_t PersistentUID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_UnRegisterPersistentUID_t)(DWord_t PersistentUID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a GATT service in the GATT database.  The     */
   /* first parameter to this function is a Boolean that specifies if   */
   /* the registered service is to be a primary service (TRUE) or a     */
   /* secondary service (FALSE).  The second parameter specifies the    */
   /* number of attributes that the service is to contain.  The third   */
   /* parameter parameter specifies the UUID of the service to register.*/
   /* The final parameter specifies an optional UUID that is used to    */
   /* ensure that a service is always given the same handle range in the*/
   /* GATT database.  This function returns a positive, non-zero, value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successful return value represents the Service ID that */
   /*          can be used to add attributes to the registered service  */
   /*          and can be used to publish the service.                  */
   /* * NOTE * A registered service is not visible to remote devices    */
   /*          until it has been published with the                     */
   /*          GATM_PublishService() API.                               */
   /* * NOTE * The NumberOfAttributes parameter must satisfy the        */
   /*          following formula:                                       */
   /*                                                                   */
   /*              NumberOfAttributes = <NumberOfIncludes>            + */
   /*                                   (<NumberOfCharacteristics>*2) + */
   /*                                   <NumberOfDescriptors>           */
   /*                                                                   */
   /*          Where                                                    */
   /*                                                                   */
   /*          <NumberOfIncludes> is the number of include              */
   /*             definitions contained in the service.                 */
   /*          <NumberOfCharacteristics> is the number of               */
   /*             characteristics contained in the service.             */
   /*          <NumberOfDescriptors> is the total number of descriptors */
   /*             contained in the service.                             */
   /*                                                                   */
   /* * NOTE * The PersistentUID parameter is optional.  If non-NULL    */
   /*          then this function will use the handle range in the GATT */
   /*          database that has been allocated for this service.  If   */
   /*          NULL any available handle range in the GATT database will*/
   /*          be used.                                                 */
BTPSAPI_DECLARATION int BTPSAPI GATM_RegisterService(Boolean_t PrimaryService, unsigned int NumberOfAttributes, GATT_UUID_t *ServiceUUID, DWord_t *PersistentUID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_RegisterService_t)(Boolean_t PrimaryService, unsigned int NumberOfAttributes, GATT_UUID_t *ServiceUUID, DWord_t *PersistentUID);
#endif

   /* The following function is used to add an Include Definition to a  */
   /* registered (but not published) GATT Service.  The first parameter */
   /* to this function is the ServiceID of the registered (but not      */
   /* published) service to add the include to.  The second is the      */
   /* Attribute Offset to place the Include Definition at in the service*/
   /* table.  The final parameter of this function is the ServiceID of  */
   /* the service that is to be include.  On success this function will */
   /* return ZERO or a negative error code on failure.                  */
   /* * NOTE * The Flags parameter is currently unused and may be       */
   /*          ignored.                                                 */
   /* * NOTE * The AttributeOffset contains the offset in the service to*/
   /*          place the include definition.  Include Definitions should*/
   /*          always be in the service table before any other          */
   /*          characteristics or descriptors.                          */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /* * NOTE * The final parameter must be the ServiceID of a service   */
   /*          that has ALREADY been registered AND published.          */
BTPSAPI_DECLARATION int BTPSAPI GATM_AddServiceInclude(unsigned long Flags, unsigned int ServiceID, unsigned int AttributeOffset, unsigned int IncludedServiceServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_AddServiceInclude_t)(unsigned long Flags, unsigned int ServiceID, unsigned int AttributeOffset, unsigned int IncludedServiceServiceID);
#endif

   /* The following function is used to add a Characteristic Declaration*/
   /* (and the Characteristic Value attribute) to a registered (but not */
   /* published) GATT Service.  The first parameter to this function is */
   /* the ServiceID of the Service to add the Characteristic Declaration*/
   /* (and the Characteristic Value) to.  The second is the Attribute   */
   /* Offset to place the Characteristic Declaration (the Characteristic*/
   /* Value will be at AttributeOffset+1).  The third parameter is the  */
   /* characteristic properties mask.  The fourth parameter is the      */
   /* Security Properties of the Characteristic.  The fifth parameter to*/
   /* this function is a pointer to the Characteristic Value UUID.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
BTPSAPI_DECLARATION int BTPSAPI GATM_AddServiceCharacteristic(unsigned int ServiceID, unsigned int AttributeOffset, unsigned long CharacteristicPropertiesMask, unsigned long SecurityPropertiesMask, GATT_UUID_t *CharacteristicUUID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_AddServiceCharacteristic_t)(unsigned int ServiceID, unsigned int AttributeOffset, unsigned long CharacteristicPropertiesMask, unsigned long SecurityPropertiesMask, GATT_UUID_t *CharacteristicUUID);
#endif

   /* The following function is used to add a Characteristic Descriptor */
   /* to a registered (but not published) GATT Service.  The first      */
   /* parameter to this function is the Service ID of the Service to add*/
   /* the Characteristic Descriptor to.  The second is the Attribute    */
   /* Offset to place the Characteristic Descriptor.  The third         */
   /* parameter is the desciptor properties mask.  The fourth parameter */
   /* is the Security Properties of the Descriptor.  The fifth parameter*/
   /* to this function is a pointer to the Descriptor UUID.  On success */
   /* this function will return ZERO or a negative error code on        */
   /* failure.                                                          */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
BTPSAPI_DECLARATION int BTPSAPI GATM_AddServiceDescriptor(unsigned int ServiceID, unsigned int AttributeOffset, unsigned long DescriptorPropertiesMask, unsigned long SecurityPropertiesMask, GATT_UUID_t *DescriptorUUID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_AddServiceDescriptor_t)(unsigned int ServiceID, unsigned int AttributeOffset, unsigned long DescriptorPropertiesMask, unsigned long SecurityPropertiesMask, GATT_UUID_t *DescriptorUUID);
#endif

   /* The following function is used to add or update semi-static data  */
   /* that is stored in a service table.  The first parameter to this   */
   /* function is the Service ID of the Service to add the semi-static  */
   /* data to.  The second parameter contains the attribute offset of   */
   /* the characteristic or descriptor to update the semi-static data   */
   /* for.  The final two parameters contain the attribute data length  */
   /* length and a pointer to the attribute data to update the          */
   /* semi-static data to.  On success this function will return ZERO or*/
   /* a negative error code on failure.                                 */
   /* * NOTE * Attribute Data can only be added to Characteristic Value */
   /*          or Characteristic Descriptor attributes.                 */
   /* * NOTE * If attribute data is added to a characteristic           */
   /*          value/descriptor value then we will respond internally to*/
   /*          read requests by client's attempting to read the value   */
   /*          (if the client that is reading the value matches the     */
   /*          specified security properties).  Write requests will     */
   /*          never be responded to internally.                        */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
BTPSAPI_DECLARATION int BTPSAPI GATM_AddServiceAttributeData(unsigned int ServiceID, unsigned int AttributeOffset, unsigned int AttributeDataLength, Byte_t *AttributeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_AddServiceAttributeData_t)(unsigned int ServiceID, unsigned int AttributeOffset, unsigned int AttributeDataLength, Byte_t *AttributeData);
#endif

   /* The following function is used to publish a previously registered */
   /* GATT Service.  The first parameter to this function is the        */
   /* ServiceID of the previously registered GATT Service.  The second  */
   /* parameter specifies the EventCallbackID of the callback that will */
   /* be dispatched events when a remote device tries to access the     */
   /* published GATT service.  The third parameter is a bit mask that is*/
   /* used to indicate properties of the service.  The final parameter  */
   /* is a pointer to a structure to store the handle range of the      */
   /* registered service if this function is successful.  On success    */
   /* this function will return ZERO or a negative error code on        */
   /* failure.                                                          */
   /* * NOTE * Once this function is called no Includes, Characteristics*/
   /*          or Descriptors can be added to the service.              */
   /* * NOTE * Once this function is called the registered service will */
   /*          be "published" and any connected Client will be able to  */
   /*          discover the service (if it is a primary service).       */
   /* * NOTE * The ServiceHandleRange parameter is optional and may be  */
   /*          set to NULL.                                             */
BTPSAPI_DECLARATION int BTPSAPI GATM_PublishService(unsigned int ServiceID, unsigned int EventCallbackID, unsigned long ServiceFlags, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_PublishService_t)(unsigned int ServiceID, unsigned int EventCallbackID, unsigned long ServiceFlags, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is used to delete a previously registered  */
   /* service.  The only parameter to this function is the Service of   */
   /* the previously registered (and/or published) GATT Service.  On    */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
   /* * NOTE * If this function is called to delete a published GATT    */
   /*          Service then connected Clients will no longer be able to */
   /*          discover the deleted GATT Service.                       */
BTPSAPI_DECLARATION int BTPSAPI GATM_DeleteService(unsigned int ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_DeleteService_t)(unsigned int ServiceID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the services that are published.  This       */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently published GATT services.  The first parameter specifies */
   /* the maximum number of GATM_Service_Information_t entries that the */
   /* buffer will support (i.e.  can be copied into the buffer).  The   */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of published services.  The next parameter  */
   /* is an optional parameter which may be used to only return services*/
   /* with a specific UUID.  The final parameter can be used to retrieve*/
   /* the total number of published services (regardless of the size of */
   /* the list specified by the first two parameters).  This function   */
   /* returns a non-negative value if successful which represents the   */
   /* number of published services that were copied into the specified  */
   /* input buffer.  This function returns a negative return error code */
   /* if there was an error.                                            */
   /* * NOTE * If ServiceUUID is NULL then any published services will  */
   /*          be returned.                                             */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Published Services Entries, in which case the final      */
   /*          parameter *MUST* be specified.                           */
BTPSAPI_DECLARATION int BTPSAPI GATM_QueryPublishedServices(unsigned int MaximumPublishedServicesListEntries, GATM_Service_Information_t *PublishedServiceList, GATT_UUID_t *ServiceUUID, unsigned int *TotalNumberPublishedServices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_QueryPublishedServices_t)(unsigned int MaximumPublishedServicesListEntries, GATM_Service_Information_t *PublishedServiceList, GATT_UUID_t *ServiceUUID, unsigned int *TotalNumberPublishedServices);
#endif

   /* The following function is used to send a Handle Value Indication  */
   /* to a specified remote device.  The first parameter is the ervice  */
   /* ID of the previously published service whose attribute is being   */
   /* indication.  The second parameter is the BD_ADDR of the connected */
   /* device to write the value to.  The third parameter contains the   */
   /* attribute offset of the value that is being indicated.  The final */
   /* two parameters contain the length of the data and a pointer to the*/
   /* data to indicate.  This function returns a positive non-zero value*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the handle value*/
   /*          confirmation that is received in response to this call.  */
BTPSAPI_DECLARATION int BTPSAPI GATM_SendHandleValueIndication(unsigned int ServiceID, BD_ADDR_t RemoteDevice, unsigned int AttributeOffset, unsigned int ValueDataLength, Byte_t *ValueData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_SendHandleValueIndication_t)(unsigned int ServiceID, BD_ADDR_t RemoteDevice, unsigned int AttributeOffset, unsigned int ValueDataLength, Byte_t *ValueData);
#endif

   /* The following function is used to send a Handle Value Notification*/
   /* to a specified remote device.  The first parameter is the Service */
   /* ID of the previously published service whose attribute is being   */
   /* indication.  The second parameter is the BD_ADDR of the connected */
   /* device to write the value to.  The third parameter contains the   */
   /* attribute offset of the value that is being indicated.  The final */
   /* two parameters contain the length of the data and a pointer to the*/
   /* data to indicate.  On success this function will return ZERO or a */
   /* negative error code on failure.                                   */
BTPSAPI_DECLARATION int BTPSAPI GATM_SendHandleValueNotification(unsigned int ServiceID, BD_ADDR_t RemoteDevice, unsigned int AttributeOffset, unsigned int ValueDataLength, Byte_t *ValueData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_SendHandleValueNotification_t)(unsigned int ServiceID, BD_ADDR_t RemoteDevice, unsigned int AttributeOffset, unsigned int ValueDataLength, Byte_t *ValueData);
#endif

   /* The following function is used to respond with success to a Write */
   /* or Prepare Write Request from a connected client.  The only       */
   /* parameter to this function is the RequestID of the request that is*/
   /* being responded to.  On success this function will return ZERO or */
   /* a negative error code on failure.                                 */
BTPSAPI_DECLARATION int BTPSAPI GATM_WriteResponse(unsigned int RequestID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_WriteResponse_t)(unsigned int RequestID);
#endif

   /* The following function is a utility function that is used to      */
   /* respond to a Read Request from a connected client.  The first     */
   /* parameter to this function is the RequestID of the request that is*/
   /* being responded to.  The final parameters to this function specify*/
   /* the length and a pointer to the data to respond with.  On success */
   /* this function will return ZERO or a negative error code on        */
   /* failure.                                                          */
BTPSAPI_DECLARATION int BTPSAPI GATM_ReadResponse(unsigned int RequestID, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_ReadResponse_t)(unsigned int RequestID, unsigned int DataLength, Byte_t *Data);
#endif

   /* The following function is a utility function that is used to      */
   /* respond with an error to a Write or a Read Requst.  The first     */
   /* parameter to this function is the RequestID of the request that is*/
   /* being responded to.  The final parameters to this function        */
   /* specifies the Attribute Protocol Error Code to respond with.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
BTPSAPI_DECLARATION int BTPSAPI GATM_ErrorResponse(unsigned int RequestID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GATM_ErrorResponse_t)(unsigned int RequestID, Byte_t ErrorCode);
#endif

#endif
