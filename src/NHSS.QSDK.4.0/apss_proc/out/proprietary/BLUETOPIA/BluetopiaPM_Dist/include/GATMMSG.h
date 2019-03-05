/*****< gatmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GATMMSG - Defined Interprocess Communication Messages for the Generic     */
/*            Attribute Profile (GATT) Manager for Stonestreet One Bluetopia  */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/16/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GATMMSGH__
#define __GATMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTGAT.h"            /* BTPS GATT Prototypes/Constants.           */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "GATMType.h"            /* BTPM GATT Manager Type Definitions.       */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Generic         */
   /* Attribute Profile (GATT) Manager.                                 */
#define BTPM_MESSAGE_GROUP_GENERIC_ATTRIBUTE_PROFILE_MANAGER         0x00001006

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Generic Attribute*/
   /* Profile (GATT) Manager.                                           */

   /* Generic Attribute Profile (GATT) Manager Connection Commands.     */
#define GATM_MESSAGE_FUNCTION_REGISTER_EVENT_CALLBACK          0x00001001
#define GATM_MESSAGE_FUNCTION_UN_REGISTER_EVENT_CALLBACK       0x00001002
#define GATM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES          0x00001003

   /* Generic Attribute Profile (GATT) Manager Client Commands.         */
#define GATM_MESSAGE_FUNCTION_READ_VALUE                       0x00001101
#define GATM_MESSAGE_FUNCTION_WRITE_VALUE                      0x00001102
#define GATM_MESSAGE_FUNCTION_WRITE_VALUE_WITHOUT_RESPONSE     0x00001103

   /* Generic Attribute Profile (GATT) Manager Server Commands.         */
#define GATM_MESSAGE_FUNCTION_REGISTER_PERSISTENT_UID          0x00001201
#define GATM_MESSAGE_FUNCTION_UN_REGISTER_PERSISTENT_UID       0x00001202
#define GATM_MESSAGE_FUNCTION_REGISTER_SERVICE                 0x00001203
#define GATM_MESSAGE_FUNCTION_ADD_SERVICE_INCLUDE              0x00001204
#define GATM_MESSAGE_FUNCTION_ADD_SERVICE_CHARACTERISTIC       0x00001205
#define GATM_MESSAGE_FUNCTION_ADD_SERVICE_DESCRIPTOR           0x00001206
#define GATM_MESSAGE_FUNCTION_ADD_SERVICE_ATTRIBUTE_DATA       0x00001207
#define GATM_MESSAGE_FUNCTION_PUBLISH_SERVICE                  0x00001208
#define GATM_MESSAGE_FUNCTION_DELETE_SERVICE                   0x00001209
#define GATM_MESSAGE_FUNCTION_QUERY_PUBLISHED_SERVICES         0x0000120A
#define GATM_MESSAGE_FUNCTION_SEND_HANDLE_VALUE_INDICATION     0x0000120B
#define GATM_MESSAGE_FUNCTION_SEND_HANDLE_VALUE_NOTIFICATION   0x0000120C
#define GATM_MESSAGE_FUNCTION_SERVER_WRITE_RESPONSE            0x0000120D
#define GATM_MESSAGE_FUNCTION_SERVER_READ_RESPONSE             0x0000120E
#define GATM_MESSAGE_FUNCTION_SERVER_ERROR_RESPONSE            0x0000120F

   /* Generic Attribute Profile (GATT) Manager Connection Events.       */
#define GATM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define GATM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002
#define GATM_MESSAGE_FUNCTION_CONNECTION_MTU_UPDATED           0x00010003
#define GATM_MESSAGE_FUNCTION_HANDLE_VALUE_DATA                0x00010004

   /* Generic Attribute Profile (GATT) Manager Client Events.           */
#define GATM_MESSAGE_FUNCTION_READ_RESPONSE                    0x00011001
#define GATM_MESSAGE_FUNCTION_WRITE_RESPONSE                   0x00011002
#define GATM_MESSAGE_FUNCTION_ERROR_RESPONSE                   0x00011003

   /* Generic Attribute Profile (GATT) Manager Server Events.           */
#define GATM_MESSAGE_FUNCTION_WRITE_REQUEST                    0x00012001
#define GATM_MESSAGE_FUNCTION_SIGNED_WRITE_COMMAND             0x00012002
#define GATM_MESSAGE_FUNCTION_READ_REQUEST                     0x00012003
#define GATM_MESSAGE_FUNCTION_PREPARE_WRITE_REQUEST            0x00012004
#define GATM_MESSAGE_FUNCTION_COMMIT_PREPARE_WRITE_EVENT       0x00012005
#define GATM_MESSAGE_FUNCTION_HANDLE_VALUE_CONFIRMATION        0x00012006

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Generic Attribute       */
   /* Profile (GATT) Manager.                                           */

   /* Generic Attribute Profile (GATT) Manager Connection               */
   /* Command/Response Message Formats.                                 */

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to register for GATT events (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_REGISTER_EVENT_CALLBACK         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Register_Event_Callback_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} GATM_Register_Event_Callback_Request_t;

#define GATM_REGISTER_EVENT_CALLBACK_REQUEST_SIZE              (sizeof(GATM_Register_Event_Callback_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to register for GATT events (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_REGISTER_EVENT_CALLBACK         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Register_Event_Callback_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          GATTEventHandlerID;
} GATM_Register_Event_Callback_Response_t;

#define GATM_REGISTER_EVENT_CALLBACK_RESPONSE_SIZE            (sizeof(GATM_Register_Event_Callback_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to un-register for GATT events (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_UN_REGISTER_EVENT_CALLBACK      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Un_Register_Event_Callback_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          GATTEventHandlerID;
} GATM_Un_Register_Event_Callback_Request_t;

#define GATM_UN_REGISTER_EVENT_CALLBACK_REQUEST_SIZE           (sizeof(GATM_Un_Register_Event_Callback_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to un-register for GATT events (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_UN_REGISTER_EVENT_CALLBACK      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Un_Register_Event_Callback_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Un_Register_Event_Callback_Response_t;

#define GATM_UN_REGISTER_EVENT_CALLBACK_RESPONSE_SIZE         (sizeof(GATM_Un_Register_Event_Callback_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to query the currently active GATT           */
   /* Connections (Request).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Query_Connected_Devices_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} GATM_Query_Connected_Devices_Request_t;

#define GATM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE              (sizeof(GATM_Query_Connected_Devices_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to query the currently active GATT           */
   /* Connections (Response).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Query_Connected_Devices_Response_t
{
   BTPM_Message_Header_t         MessageHeader;
   int                           Status;
   unsigned int                  NumberConnectedDevices;
   GATM_Connection_Information_t ConnectionList[1];
} GATM_Query_Connected_Devices_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Query Connected Devices      */
   /* Response Message given the number of GATT Connections.  This      */
   /* function accepts as it's input the total number of GATT           */
   /* Connections (NOT bytes) that are present starting from the        */
   /* ConnectionList member of the                                      */
   /* GATM_Query_Connected_Devices_Response_t structure and returns the */
   /* total number of bytes required to hold the entire message.        */
#define GATM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(_x)         (STRUCTURE_OFFSET(GATM_Query_Connected_Devices_Response_t, ConnectionList) + (unsigned int)((sizeof(GATM_Connection_Information_t)*((unsigned int)(_x)))))

   /* Generic Attribute Profile (GATT) Manager Client Command/Response  */
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to read an attribute value from a remote     */
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_READ_VALUE                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Read_Value_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          GATTEventHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Word_t                AttributeHandle;
   unsigned int          Offset;
   Boolean_t             ReadAll;
} GATM_Read_Value_Request_t;

#define GATM_READ_VALUE_REQUEST_SIZE                           (sizeof(GATM_Read_Value_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to read an attribute value from a remote     */
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_READ_VALUE                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Read_Value_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} GATM_Read_Value_Request_Response_t;

#define GATM_READ_VALUE_REQUEST_RESPONSE_SIZE                  (sizeof(GATM_Read_Value_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to write an attribute value to a remote      */
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_WRITE_VALUE                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Value_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   BD_ADDR_t              RemoteDeviceAddress;
   Word_t                 AttributeHandle;
   unsigned int           ValueDataLength;
   Byte_t                 ValueData[1];
} GATM_Write_Value_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Write Value Request Message  */
   /* given the number of actual bytes to write.  This function accepts */
   /* as it's input the total number individual data bytes are present  */
   /* starting from the ValueData member of the                         */
   /* GATM_Write_Value_Request_t structure and returns the total number */
   /* of bytes required to hold the entire message.                     */
#define GATM_WRITE_VALUE_REQUEST_SIZE(_x)                      (STRUCTURE_OFFSET(GATM_Write_Value_Request_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to write an attribute value to a remote      */
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_WRITE_VALUE                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Value_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} GATM_Write_Value_Request_Response_t;

#define GATM_WRITE_VALUE_REQUEST_RESPONSE_SIZE                 (sizeof(GATM_Write_Value_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to write an attribute value to a remote      */
   /* device without response (Request).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_WRITE_VALUE_WITHOUT_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Value_Without_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          GATTEventHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Word_t                AttributeHandle;
   Boolean_t             PerfomSignedWrite;
   unsigned int          SignatureCounter;
   unsigned int          ValueDataLength;
   Byte_t                ValueData[1];
} GATM_Write_Value_Without_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Write Value Without Response */
   /* Request Message given the number of actual bytes to write.  This  */
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the ValueData member of the       */
   /* GATM_Write_Value_Without_Response_Request_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define GATM_WRITE_VALUE_WITHOUT_RESPONSE_REQUEST_SIZE(_x)     (STRUCTURE_OFFSET(GATM_Write_Value_Without_Response_Request_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to write an attribute value to a remote      */
   /* device without response (Response).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_WRITE_VALUE_WITHOUT_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Value_Without_Response_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          BytesWritten;
} GATM_Write_Value_Without_Response_Request_Response_t;

#define GATM_WRITE_VALUE_WITHOUT_RESPONSE_REQUEST_RESPONSE_SIZE    (sizeof(GATM_Write_Value_Without_Response_Request_Response_t))

   /* Generic Attribute Profile (GATT) Manager Server Command/Response  */
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to register a persistent UID (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_REGISTER_PERSISTENT_UID         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Register_Persistent_UID_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          NumberOfAttributes;
} GATM_Register_Persistent_UID_Request_t;

#define GATM_REGISTER_PERSISTENT_UID_REQUEST_SIZE              (sizeof(GATM_Register_Persistent_UID_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to register a persistent UID (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_REGISTER_PERSISTENT_UID         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Register_Persistent_UID_Response_t
{
   BTPM_Message_Header_t         MessageHeader;
   int                           Status;
   DWord_t                       PersistentUID;
   GATT_Attribute_Handle_Group_t ServiceHandleRangeResult;
} GATM_Register_Persistent_UID_Response_t;

#define GATM_REGISTER_PERSISTENT_UID_RESPONSE_SIZE            (sizeof(GATM_Register_Persistent_UID_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to un-register a persistent UID (Request).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_UN_REGISTER_PERSISTENT_UID      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Un_Register_Persistent_UID_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   DWord_t               PersistentUID;
} GATM_Un_Register_Persistent_UID_Request_t;

#define GATM_UN_REGISTER_PERSISTENT_UID_REQUEST_SIZE          (sizeof(GATM_Un_Register_Persistent_UID_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to un-register a persistent UID (Response).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_UN_REGISTER_PERSISTENT_UID      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Un_Register_Persistent_UID_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Un_Register_Persistent_UID_Response_t;

#define GATM_UN_REGISTER_PERSISTENT_UID_RESPONSE_SIZE         (sizeof(GATM_Un_Register_Persistent_UID_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to register a service (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_REGISTER_SERVICE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Register_Service_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         RegisterServiceFlags;
   unsigned int          NumberOfAttributes;
   GATT_UUID_t           ServiceUUID;
   DWord_t               PersistentUID;
} GATM_Register_Service_Request_t;

#define GATM_REGISTER_SERVICE_REQUEST_SIZE                     (sizeof(GATM_Register_Service_Request_t))

   /* The following define the flags that are valid with the            */
   /* RegisterServiceFlags member of the GATM_Register_Service_Request_t*/
   /* structure.                                                        */
#define GATM_REGISTER_SERVICE_FLAGS_PRIMARY_SERVICE            0x00000001
#define GATM_REGISTER_SERVICE_FLAGS_PERSISTENT_UID_VALID       0x00000002

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to register a service (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_REGISTER_SERVICE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Register_Service_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ServiceID;
} GATM_Register_Service_Request_Response_t;

#define GATM_REGISTER_SERVICE_REQUEST_RESPONSE_SIZE            (sizeof(GATM_Register_Service_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add an include definition to a service    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_INCLUDE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Include_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         Flags;
   unsigned int          ServiceID;
   unsigned int          AttributeOffset;
   unsigned int          IncludedServiceServiceID;
} GATM_Add_Service_Include_Request_t;

#define GATM_ADD_SERVICE_INCLUDE_REQUEST_SIZE                  (sizeof(GATM_Add_Service_Include_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add an include definition to a service    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_INCLUDE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Include_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Add_Service_Include_Request_Response_t;

#define GATM_ADD_SERVICE_INCLUDE_REQUEST_RESPONSE_SIZE         (sizeof(GATM_Add_Service_Include_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add a characteristic to a service         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_CHARACTERISTIC      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Characteristic_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
   unsigned int          AttributeOffset;
   unsigned long         CharacteristicPropertiesMask;
   unsigned long         SecurityPropertiesMask;
   GATT_UUID_t           CharacteristicUUID;
} GATM_Add_Service_Characteristic_Request_t;

#define GATM_ADD_SERVICE_CHARACTERISTIC_REQUEST_SIZE           (sizeof(GATM_Add_Service_Characteristic_Request_t))

   /* The following define the flags that are valid with the            */
   /* SecurityProperties member of the                                  */
   /* GATM_Add_Service_Characteristic_Request_t and                     */
   /* GATM_Add_Service_Descriptor_Request_t structures.                 */
#define GATM_SECURITY_PROPERTIES_NO_SECURITY                         0x00000000
#define GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_WRITE    0x00000001
#define GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_WRITE      0x00000002
#define GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_ENCRYPTION_READ     0x00000004
#define GATM_SECURITY_PROPERTIES_AUTHENTICATED_ENCRYPTION_READ       0x00000008
#define GATM_SECURITY_PROPERTIES_UNAUTHENTICATED_SIGNED_WRITES       0x00000010
#define GATM_SECURITY_PROPERTIES_AUTHENTICATED_SIGNED_WRITES         0x00000020

   /* The following define the flags that are valid with the            */
   /* CharacteristicProperties member of the                            */
   /* GATM_Add_Service_Characteristic_Request_t structure.              */
#define GATM_CHARACTERISTIC_PROPERTIES_BROADCAST                     0x00000001
#define GATM_CHARACTERISTIC_PROPERTIES_READ                          0x00000002
#define GATM_CHARACTERISTIC_PROPERTIES_WRITE_WO_RESP                 0x00000004
#define GATM_CHARACTERISTIC_PROPERTIES_WRITE                         0x00000008
#define GATM_CHARACTERISTIC_PROPERTIES_NOTIFY                        0x00000010
#define GATM_CHARACTERISTIC_PROPERTIES_INDICATE                      0x00000020
#define GATM_CHARACTERISTIC_PROPERTIES_AUTHENTICATED_SIGNED_WRITES   0x00000040
#define GATM_CHARACTERISTIC_PROPERTIES_EXT_PROPERTIES                0x00000080

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add a characteristic to a service         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_CHARACTERISTIC      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Characteristic_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Add_Service_Characteristic_Request_Response_t;

#define GATM_ADD_SERVICE_CHARACTERISTIC_REQUEST_RESPONSE_SIZE  (sizeof(GATM_Add_Service_Characteristic_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add a descriptor to a service (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_DESCRIPTOR          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Descriptor_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
   unsigned int          AttributeOffset;
   unsigned int          DescriptorPropertiesMask;
   unsigned int          SecurityPropertiesMask;
   GATT_UUID_t           DescriptorUUID;
} GATM_Add_Service_Descriptor_Request_t;

#define GATM_ADD_SERVICE_DESCRIPTOR_REQUEST_SIZE               (sizeof(GATM_Add_Service_Descriptor_Request_t))

   /* The following define the flags that are valid with the            */
   /* DescriptorProperties member of the GATM_Add_Descriptor_Request_t  */
   /* structure.                                                        */
#define GATM_DESCRIPTOR_PROPERTIES_READ                    0x00000001
#define GATM_DESCRIPTOR_PROPERTIES_WRITE                   0x00000002

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add a descriptor to a service (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_DESCRIPTOR          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Descriptor_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Add_Service_Descriptor_Request_Response_t;

#define GATM_ADD_SERVICE_DESCRIPTOR_REQUEST_RESPONSE_SIZE      (sizeof(GATM_Add_Service_Descriptor_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add semi static data to a characteristic  */
   /* value or characteristic descriptor (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_ATTRIBUTE_DATA      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Attribute_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
   unsigned int          AttributeOffset;
   unsigned int          AttributeDataLength;
   Byte_t                AttributeData[1];
} GATM_Add_Service_Attribute_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Add Attribute Data Request   */
   /* Message given the number of actual bytes to write bytes.  This    */
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the AttributeData member of the   */
   /* GATM_Add_Attribute_Data_Request_t structure and returns the total */
   /* number of bytes required to hold the entire message.              */
#define GATM_ADD_SERVICE_ATTRIBUTE_DATA_REQUEST_SIZE(_x)       (STRUCTURE_OFFSET(GATM_Add_Service_Attribute_Data_Request_t, AttributeData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to add semi static data to a characteristic  */
   /* value or characteristic descriptor (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ADD_SERVICE_ATTRIBUTE_DATA      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Add_Service_Attribute_Data_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Add_Service_Attribute_Data_Request_Response_t;

#define GATM_ADD_SERVICE_ATTRIBUTE_DATA_REQUEST_RESPONSE_SIZE  (sizeof(GATM_Add_Service_Attribute_Data_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to publish a previously registered service   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_PUBLISH_SERVICE                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Publish_Service_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
   unsigned int          GATTEventHandlerID;
   unsigned long         ServiceFlags;
} GATM_Publish_Service_Request_t;

#define GATM_PUBLISH_SERVICE_REQUEST_SIZE                      (sizeof(GATM_Publish_Service_Request_t))

   /* The following define the flags that are valid with the            */
   /* ServiceFlags member of the GATM_Publish_Service_Request_t         */
   /* structure.                                                        */
#define GATM_SERVICE_FLAGS_SUPPORT_LOW_ENERGY                  0x00000001
#define GATM_SERVICE_FLAGS_SUPPORT_CLASSIC_BLUETOOTH           0x00000002

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to publish a previously registered service   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_PUBLISH_SERVICE                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Publish_Service_Request_Response_t
{
   BTPM_Message_Header_t         MessageHeader;
   int                           Status;
   GATT_Attribute_Handle_Group_t ServiceHandleRange;
} GATM_Publish_Service_Request_Response_t;

#define GATM_PUBLISH_SERVICE_REQUEST_RESPONSE_SIZE             (sizeof(GATM_Publish_Service_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to delete a previously registered (and/or    */
   /* published) service (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_DELETE_SERVICE                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Delete_Service_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
} GATM_Delete_Service_Request_t;

#define GATM_DELETE_SERVICE_REQUEST_SIZE                       (sizeof(GATM_Delete_Service_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to delete a previously registered (and/or    */
   /* published) service (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_DELETE_SERVICE                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Delete_Service_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Delete_Service_Request_Response_t;

#define GATM_DELETE_SERVICE_REQUEST_RESPONSE_SIZE              (sizeof(GATM_Delete_Service_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to query the currently published GATT        */
   /* Services (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_QUERY_PUBLISHED_SERVICES        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Query_Published_Services_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         QueryPublishedServicesFlags;
   GATT_UUID_t           ServiceUUID;
} GATM_Query_Published_Services_Request_t;

#define GATM_QUERY_PUBLISHED_SERVICES_REQUEST_SIZE             (sizeof(GATM_Query_Published_Services_Request_t))

   /* The following define the flags that are valid with the            */
   /* QueryPublishedServicesFlags member of the                         */
   /* GATM_Query_Published_Services_Request_t structure.                */
#define GATM_QUERY_PUBLISHED_SERVICES_FLAGS_UUID_VALID          0x00000001

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to query the currently published GATT        */
   /* Services (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_QUERY_PUBLISHED_SERVICES        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Query_Published_Services_Request_Response_t
{
   BTPM_Message_Header_t      MessageHeader;
   int                        Status;
   unsigned int               NumberPublishedServices;
   GATM_Service_Information_t PublishedServiceList[1];
} GATM_Query_Published_Services_Request_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Query Published Services     */
   /* Response Message given the number of Published Services.  This    */
   /* function accepts as it's input the total number of Published      */
   /* Services (NOT bytes) that are present starting from the           */
   /* PublishedServiceList member of the                                */
   /* GATM_Query_Published_Services_Response_t structure and returns the*/
   /* total number of bytes required to hold the entire message.        */
#define GATM_QUERY_PUBLISHED_SERVICES_REQUEST_RESPONSE_SIZE(_x)   (STRUCTURE_OFFSET(GATM_Query_Published_Services_Request_Response_t, PublishedServiceList) + (unsigned int)((sizeof(GATM_Service_Information_t)*((unsigned int)(_x)))))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to send a Handle Value Indication to a remote*/
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SEND_HANDLE_VALUE_INDICATION    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Send_Handle_Value_Indication_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          AttributeOffset;
   unsigned int          ValueDataLength;
   Byte_t                ValueData[1];
} GATM_Send_Handle_Value_Indication_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Handle Value Indication      */
   /* Request Message given the number of actual value data bytes.  This*/
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the ValueData member of the       */
   /* GATM_Handle_Value_Indication_Request_t structure and returns the  */
   /* total number of bytes required to hold the entire message.        */
#define GATM_SEND_HANDLE_VALUE_INDICATION_REQUEST_SIZE(_x)     (STRUCTURE_OFFSET(GATM_Send_Handle_Value_Indication_Request_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to send a Handle Value Indication to a remote*/
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SEND_HANDLE_VALUE_INDICATION    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Send_Handle_Value_Indication_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} GATM_Send_Handle_Value_Indication_Request_Response_t;

#define GATM_SEND_HANDLE_VALUE_INDICATION_REQUEST_RESPONSE_SIZE   (sizeof(GATM_Send_Handle_Value_Indication_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to send a Handle Value Notification to a     */
   /* remote device (Request).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SEND_HANDLE_VALUE_NOTIFICATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Send_Handle_Value_Notification_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServiceID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          AttributeOffset;
   unsigned int          ValueDataLength;
   Byte_t                ValueData[1];
} GATM_Send_Handle_Value_Notification_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Handle Value Notification    */
   /* Request Message given the number of value data bytes.  This       */
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the ValueData member of the       */
   /* GATM_Handle_Value_Notification_Request_t structure and returns the*/
   /* total number of bytes required to hold the entire message.        */
#define GATM_SEND_HANDLE_VALUE_NOTIFICATION_REQUEST_SIZE(_x)   (STRUCTURE_OFFSET(GATM_Send_Handle_Value_Notification_Request_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to send a Handle Value Notification to a     */
   /* remote device (Response).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SEND_HANDLE_VALUE_NOTIFICATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Send_Handle_Value_Notification_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Send_Handle_Value_Notification_Request_Response_t;

#define GATM_SEND_HANDLE_VALUE_NOTIFICATION_REQUEST_RESPONSE_SIZE  (sizeof(GATM_Send_Handle_Value_Notification_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to respond successfully to a Write or a      */
   /* Prepare Write Request (Request).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SERVER_WRITE_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RequestID;
} GATM_Write_Response_Request_t;

#define GATM_WRITE_RESPONSE_REQUEST_SIZE                       (sizeof(GATM_Write_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to respond successfully to a Write or a      */
   /* Prepare Write Request (Response).                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SERVER_WRITE_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Response_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Write_Response_Request_Response_t;

#define GATM_WRITE_RESPONSE_REQUEST_RESPONSE_SIZE              (sizeof(GATM_Write_Response_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to respond to a Read Request (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SERVER_READ_RESPONSE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Read_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RequestID;
   unsigned int          ValueLength;
   Byte_t                Value[1];
} GATM_Read_Response_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Read Response Request Message*/
   /* given the number of value bytes.  This function accepts as it's   */
   /* input the total number individual data bytes are present starting */
   /* from the Value member of the GATM_Read_Response_Request_t         */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define GATM_READ_RESPONSE_REQUEST_SIZE(_x)                    (STRUCTURE_OFFSET(GATM_Read_Response_Request_t, Value) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to respond to a Read Request (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SERVER_READ_RESPONSE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Read_Response_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Read_Response_Request_Response_t;

#define GATM_READ_RESPONSE_REQUEST_RESPONSE_SIZE             (sizeof(GATM_Read_Response_Request_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to respond with an error to a server request */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SERVER_ERROR_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Error_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RequestID;
   Byte_t                ErrorCode;
} GATM_Error_Response_Request_t;

#define GATM_ERROR_RESPONSE_REQUEST_SIZE                       (sizeof(GATM_Error_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATT Manager Message to respond with an error to a server request */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SERVER_ERROR_RESPONSE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Error_Response_Request_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GATM_Error_Response_Request_Response_t;

#define GATM_ERROR_RESPONSE_REQUEST_RESPONSE_SIZE              (sizeof(GATM_Error_Response_Request_Response_t))

   /* GATT Manager Asynchronous Connection Message Formats.             */

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that informs the local device that a remote  */
   /* GATT connection has been made (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Word_t                 MTU;
} GATM_Connected_Message_t;

#define GATM_CONNECTED_MESSAGE_SIZE                            (sizeof(GATM_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that informs the local device that a remote  */
   /* GATT connection has been disconnected (asynchronously)            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} GATM_Disconnected_Message_t;

#define GATM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(GATM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that informs the local device that a remote  */
   /* GATT connection has an updated MTU (asynchronously).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_CONNECTION_MTU_UPDATED          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Connection_MTU_Update_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Word_t                 MTU;
} GATM_Connection_MTU_Update_Message_t;

#define GATM_CONNECTION_MTU_UPDATE_MESSAGE_SIZE                (sizeof(GATM_Connection_MTU_Update_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a Handle Value Indication or Notification to the local      */
   /* device (asynchronously).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_HANDLE_VALUE_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Handle_Value_Data_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   Boolean_t              HandleValueIndication;
   Word_t                 AttributeHandle;
   unsigned int           ValueDataLength;
   Byte_t                 ValueData[1];
} GATM_Handle_Value_Data_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Handle Value Data Message    */
   /* given the number of actual value data bytes.  This function       */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the ValueData member of the                 */
   /* GATM_Handle_Value_Data_Message_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
#define GATM_HANDLE_VALUE_DATA_MESSAGE_SIZE(_x)                (STRUCTURE_OFFSET(GATM_Handle_Value_Data_Message_t, ValueData) + ((unsigned int)(_x)))

   /* GATT Manager Asynchronous Client Message Formats.                 */

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a response to a read issued by the local device             */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_READ_RESPONSE                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Read_Response_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           TransactionID;
   Word_t                 Handle;
   Boolean_t              Final;
   unsigned int           ValueDataLength;
   Byte_t                 ValueData[1];
} GATM_Read_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Read Response Message given  */
   /* the number of actual value data bytes.  This function accepts as  */
   /* it's input the total number individual data bytes are present     */
   /* starting from the ValueData member of the GATM_Read_Response_t    */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define GATM_READ_RESPONSE_SIZE(_x)                            (STRUCTURE_OFFSET(GATM_Read_Response_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a response to a write issued by the local device            */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_WRITE_RESPONSE                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Response_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           TransactionID;
   Word_t                 Handle;
} GATM_Write_Response_t;

#define GATM_WRITE_RESPONSE_SIZE                               (sizeof(GATM_Write_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends an error response to a Read or Write request issued by the  */
   /* local device (asynchronously).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_ERROR_RESPONSE                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Error_Response_t
{
   BTPM_Message_Header_t     MessageHeader;
   unsigned int              GATTEventHandlerID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDeviceAddress;
   unsigned int              TransactionID;
   Word_t                    Handle;
   GATT_Request_Error_Type_t ErrorType;
   Byte_t                    AttributeProtocolErrorCode;
} GATM_Error_Response_t;

#define GATM_ERROR_RESPONSE_SIZE                               (sizeof(GATM_Error_Response_t))

   /* GATT Manager Asynchronous Server Message Formats.                 */

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a write request to the local device (asynchronously).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_WRITE_REQUEST                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Write_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   unsigned int           RequestID;
   unsigned int           AttributeOffset;
   unsigned int           ValueDataLength;
   Byte_t                 ValueData[1];
} GATM_Write_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Write Request Message given  */
   /* the number of actual value data bytes.  This function accepts as  */
   /* it's input the total number individual data bytes are present     */
   /* starting from the ValueData member of the GATM_Write_Request_t    */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define GATM_WRITE_REQUEST_SIZE(_x)                            (STRUCTURE_OFFSET(GATM_Write_Request_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a signed write command to the local device (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_SIGNED_WRITE_COMMAND            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Signed_Write_Command_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   Boolean_t              ValidSignature;
   unsigned int           AttributeOffset;
   unsigned int           ValueDataLength;
   Byte_t                 ValueData[1];
} GATM_Signed_Write_Command_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Signed Write Request Message */
   /* given the number of actual value data bytes.  This function       */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the ValueData member of the                 */
   /* GATM_Signed_Write_Request_t structure and returns the total number*/
   /* of bytes required to hold the entire message.                     */
#define GATM_SIGNED_WRITE_COMMAND_SIZE(_x)                     (STRUCTURE_OFFSET(GATM_Signed_Write_Command_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a read request to the local device (asynchronously).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_READ_REQUEST                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Read_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   unsigned int           RequestID;
   unsigned int           AttributeOffset;
   unsigned int           AttributeValueOffset;
} GATM_Read_Request_t;

#define GATM_READ_REQUEST_SIZE                                 (sizeof(GATM_Read_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* does a prepare write procedure to the local device                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_PREPARE_WRITE_REQUEST           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Prepare_Write_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   unsigned int           RequestID;
   unsigned int           AttributeOffset;
   unsigned int           AttributeValueOffset;
   unsigned int           ValueDataLength;
   Byte_t                 ValueData[1];
} GATM_Prepare_Write_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Prepare Write Request Message*/
   /* given the number of actual value data bytes.  This function       */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the ValueData member of the                 */
   /* GATM_Prepare_Write_Request_t structure and returns the total      */
   /* number of bytes required to hold the entire message.              */
#define GATM_PREPARE_WRITE_REQUEST_SIZE(_x)                    (STRUCTURE_OFFSET(GATM_Prepare_Write_Request_t, ValueData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when prepare writes for   */
   /* the specified device may either be committed or de-committed      */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_COMMIT_PREPARE_WRITE_EVENT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Commit_Prepare_Write_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   Boolean_t              CommitWrites;
} GATM_Commit_Prepare_Write_Message_t;

#define GATM_COMMIT_PREPARE_WRITE_MESSAGE_SIZE                 (sizeof(GATM_Commit_Prepare_Write_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GATM Manager Message that is dispatched when the remote device    */
   /* sends a Handle Value Confirmation to a Handle Value Indication    */
   /* sent by the local device (asynchronously).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GATM_MESSAGE_FUNCTION_HANDLE_VALUE_CONFIRMATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGATM_Handle_Value_Confirmation_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           GATTEventHandlerID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           ServiceID;
   unsigned int           TransactionID;
   unsigned int           AttributeOffset;
   unsigned int           Status;
} GATM_Handle_Value_Confirmation_t;

#define GATM_HANDLE_VALUE_CONFIRMATION_SIZE                    (sizeof(GATM_Handle_Value_Confirmation_t))

   /* The following constants represent the server confirmation status  */
   /* values that are possible in the Status member of the              */
   /* GATM_Handle_Value_Confirmation_t structure.                       */
#define GATM_HANDLE_VALUE_CONFIRMATION_STATUS_SUCCESS          0x00000000
#define GATM_HANDLE_VALUE_CONFIRMATION_STATUS_TIMEOUT          0x00000001

#endif
