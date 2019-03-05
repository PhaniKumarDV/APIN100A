/*****< devmmsg.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DEVMMSG - Defined Interprocess Communication Messages for the Local       */
/*            Device Manager for Stonestreet One Bluetopia Protocol Stack     */
/*            Platform Manager.                                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __DEVMMSGH__
#define __DEVMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "DEVMType.h"            /* BTPM Device Manager Type Definitions.     */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Local Device    */
   /* Manager.                                                          */
#define BTPM_MESSAGE_GROUP_LOCAL_DEVICE_MANAGER                            0x00000101

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Local Device     */
   /* Manager.                                                          */

   /* Local Device Manager Commands.                                    */
#define DEVM_MESSAGE_FUNCTION_ENABLE_DISABLE_DEVICE                        0x00001001
#define DEVM_MESSAGE_FUNCTION_QUERY_DEVICE_ENABLE                          0x00001002
#define DEVM_MESSAGE_FUNCTION_QUERY_LOCAL_DEVICE_PROPERTIES                0x00001003
#define DEVM_MESSAGE_FUNCTION_UPDATE_LOCAL_DEVICE_PROPERTIES               0x00001004
#define DEVM_MESSAGE_FUNCTION_QUERY_LOCAL_DEVICE_ID_INFORMATION            0x00001005
#define DEVM_MESSAGE_FUNCTION_ADD_INTERNAL_EIR_DATA                        0x00001006
#define DEVM_MESSAGE_FUNCTION_ADD_LOCAL_EIR_DATA                           0x00001007

#define DEVM_MESSAGE_FUNCTION_START_DEVICE_DISCOVERY                       0x00001101
#define DEVM_MESSAGE_FUNCTION_STOP_DEVICE_DISCOVERY                        0x00001102
#define DEVM_MESSAGE_FUNCTION_START_DEVICE_SCAN                            0x00001103
#define DEVM_MESSAGE_FUNCTION_STOP_DEVICE_SCAN                             0x00001104
#define DEVM_MESSAGE_FUNCTION_START_ADVERTISING                            0x00001105
#define DEVM_MESSAGE_FUNCTION_STOP_ADVERTISING                             0x00001106
#define DEVM_MESSAGE_FUNCTION_START_OBSERVATION_SCAN                       0x00001107
#define DEVM_MESSAGE_FUNCTION_STOP_OBSERVATION_SCAN                        0x00001108
#define DEVM_MESSAGE_FUNCTION_SCHEDULE_INTERLEAVED_ADVERTISEMENT           0x00001109
#define DEVM_MESSAGE_FUNCTION_CANCEL_INTERLEAVED_ADVERTISEMENT             0x0000110A
#define DEVM_MESSAGE_FUNCTION_SUSPEND_INTERLEAVED_ADVERTISEMENTS           0x0000110B
#define DEVM_MESSAGE_FUNCTION_RESUME_INTERLEAVED_ADVERTISEMENTS            0x0000110C

#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_LIST                     0x00001201
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_PROPERTIES               0x00001202
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICES                 0x00001203
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICE_SUPPORTED        0x00001204
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICES_FOR_SERVICE             0x00001205
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICE_CLASSES          0x00001206
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_EIR_DATA                 0x00001207
#define DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ADVERTISING_DATA         0x00001208

#define DEVM_MESSAGE_FUNCTION_ADD_REMOTE_DEVICE                            0x00001301
#define DEVM_MESSAGE_FUNCTION_DELETE_REMOTE_DEVICE                         0x00001302
#define DEVM_MESSAGE_FUNCTION_UPDATE_REMOTE_DEVICE_APP_DATA                0x00001303
#define DEVM_MESSAGE_FUNCTION_DELETE_REMOTE_DEVICES                        0x00001304
#define DEVM_MESSAGE_FUNCTION_PAIR_WITH_REMOTE_DEVICE                      0x00001305
#define DEVM_MESSAGE_FUNCTION_CANCEL_PAIR_WITH_REMOTE_DEVICE               0x00001306
#define DEVM_MESSAGE_FUNCTION_UNPAIR_REMOTE_DEVICE                         0x00001307
#define DEVM_MESSAGE_FUNCTION_AUTHENTICATE_REMOTE_DEVICE                   0x00001308
#define DEVM_MESSAGE_FUNCTION_ENCRYPT_REMOTE_DEVICE                        0x00001309
#define DEVM_MESSAGE_FUNCTION_CONNECT_WITH_REMOTE_DEVICE                   0x0000130A
#define DEVM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE                     0x0000130B
#define DEVM_MESSAGE_FUNCTION_SET_REMOTE_DEVICE_LINK_ACTIVE                0x0000130C

#define DEVM_MESSAGE_FUNCTION_CREATE_SERVICE_RECORD                        0x00001400
#define DEVM_MESSAGE_FUNCTION_DELETE_SERVICE_RECORD                        0x00001401
#define DEVM_MESSAGE_FUNCTION_ADD_SERVICE_RECORD_ATTRIBUTE                 0x00001402
#define DEVM_MESSAGE_FUNCTION_DELETE_SERVICE_RECORD_ATTRIBUTE              0x00001403

#define DEVM_MESSAGE_FUNCTION_ENABLE_BLUETOOTH_DEBUG                       0x00001501

#define DEVM_MESSAGE_FUNCTION_REGISTER_AUTHENTICATION                      0x00002001
#define DEVM_MESSAGE_FUNCTION_UNREGISTER_AUTHENTICATION                    0x00002002
#define DEVM_MESSAGE_FUNCTION_AUTHENTICATION_RESPONSE                      0x00002003

#define DEVM_MESSAGE_FUNCTION_SEND_RAW_HCI_COMMAND                         0x00002101

#define DEVM_MESSAGE_FUNCTION_ENABLE_LOCAL_DEVICE_FEATURE                  0x00002201
#define DEVM_MESSAGE_FUNCTION_DISABLE_LOCAL_DEVICE_FEATURE                 0x00002202
#define DEVM_MESSAGE_FUNCTION_QUERY_ACTIVE_LOCAL_DEVICE_FEATURES           0x00002203

#define DEVM_MESSAGE_FUNCTION_ROLE_SWITCH                                  0x00002301

   /* Local Device Manager Asynchronous Events.                         */
#define DEVM_MESSAGE_FUNCTION_DEVICE_ENABLE_DISABLE_CHANGE                 0x00010001
#define DEVM_MESSAGE_FUNCTION_LOCAL_DEVICE_PROPERTIES_CHANGED              0x00010002
#define DEVM_MESSAGE_FUNCTION_DEVICE_DISCOVERY_STARTED                     0x00010003
#define DEVM_MESSAGE_FUNCTION_DEVICE_DISCOVERY_STOPPED                     0x00010004
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_FOUND                          0x00010005
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_DELETED                        0x00010006
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_PROPERTIES_CHANGED             0x00010007
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_PROPERTIES_STATUS              0x00010008
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_SERVICES_STATUS                0x00010009
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_PAIRING_STATUS                 0x0001000A
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_AUTHENT_STATUS                 0x0001000B
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_ENCRYPTION_STATUS              0x0001000C
#define DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_CONNECTION_STATUS              0x0001000D
#define DEVM_MESSAGE_FUNCTION_DEVICE_SCAN_STARTED                          0x0001000E
#define DEVM_MESSAGE_FUNCTION_DEVICE_SCAN_STOPPED                          0x0001000F
#define DEVM_MESSAGE_FUNCTION_DEVICE_ADDRESS_CHANGED                       0x00010010
#define DEVM_MESSAGE_FUNCTION_DEVICE_ADVERTISING_STARTED                   0x00010011
#define DEVM_MESSAGE_FUNCTION_DEVICE_ADVERTISING_STOPPED                   0x00010012
#define DEVM_MESSAGE_FUNCTION_ADVERTISING_TIMEOUT                          0x00010013
#define DEVM_MESSAGE_FUNCTION_DEVICE_OBSERVATION_SCAN_STARTED              0x00010014
#define DEVM_MESSAGE_FUNCTION_DEVICE_OBSERVATION_SCAN_STOPPED              0x00010015
#define DEVM_MESSAGE_FUNCTION_DEVICE_INTERLEAVED_ADVERTISEMENT_COMPLETE    0x00010016
#define DEVM_MESSAGE_FUNCTION_DEVICE_INTERLEAVED_ADVERTISING_SUSPENDED     0x00010017
#define DEVM_MESSAGE_FUNCTION_DEVICE_INTERLEAVED_ADVERTISING_RESUMED       0x00010018

#define DEVM_MESSAGE_FUNCTION_AUTHENTICATION_EVENT                         0x00011001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Local Device Manager.   */

   /* Local Device Manager Command/Response Message Formats.            */

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Enable/Disable the local Device         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENABLE_DISABLE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Enable_Disable_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   Boolean_t             Enable;
} DEVM_Enable_Disable_Device_Request_t;

#define DEVM_ENABLE_DISABLE_DEVICE_REQUEST_SIZE                (sizeof(DEVM_Enable_Disable_Device_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Enable/Disable the local Device         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENABLE_DISABLE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Enable_Disable_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Enable_Disable_Device_Response_t;

#define DEVM_ENABLE_DISABLE_DEVICE_RESPONSE_SIZE               (sizeof(DEVM_Enable_Disable_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query if the local device is Enabled or */
   /* Disabled (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_DEVICE_ENABLE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Device_Enable_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Query_Device_Enable_Request_t;

#define DEVM_QUERY_DEVICE_ENABLE_REQUEST_SIZE                  (sizeof(DEVM_Query_Device_Enable_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query if the local device is Enabled or */
   /* Disabled (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_DEVICE_ENABLE             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Device_Enable_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          EnableState;
} DEVM_Query_Device_Enable_Response_t;

#define DEVM_QUERY_DEVICE_ENABLE_RESPONSE_SIZE                 (sizeof(DEVM_Query_Device_Enable_Response_t))

   /* The following constants are used with the EnableState member of   */
   /* the DEVM_Query_Device_Enable_Response_t and                       */
   /* DEVM_Device_Enable_Disable_Change_Message_t message to reflect the*/
   /* current Enable/Disable state.  These values mean the following:   */
   /*    - DISABLED    - Device is now longer enabled                   */
   /*    - ENABLED     - Device is now fully enabled                    */
   /*    - PRE_DISABLE - Device is about to be disabled                 */
#define DEVM_DEVICE_ENABLE_DISABLE_ENABLE_STATE_DISABLED       0x00000000
#define DEVM_DEVICE_ENABLE_DISABLE_ENABLE_STATE_ENABLED        0x00000001
#define DEVM_DEVICE_ENABLE_DISABLE_ENABLE_STATE_PRE_DISABLE    0x00000002

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Local Device Properties       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_LOCAL_DEVICE_PROPERTIES   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Local_Device_Properties_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Query_Local_Device_Properties_Request_t;

#define DEVM_QUERY_LOCAL_DEVICE_PROPERTIES_REQUEST_SIZE        (sizeof(DEVM_Query_Local_Device_Properties_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Local Device Properties       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_LOCAL_DEVICE_PROPERTIES   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Local_Device_Properties_Response_t
{
   BTPM_Message_Header_t          MessageHeader;
   int                            Status;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;
} DEVM_Query_Local_Device_Properties_Response_t;

#define DEVM_QUERY_LOCAL_DEVICE_PROPERTIES_RESPONSE_SIZE       (sizeof(DEVM_Query_Local_Device_Properties_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Update the Local Device Properties      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UPDATE_LOCAL_DEVICE_PROPERTIES  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Update_Local_Device_Properties_Request_t
{
   BTPM_Message_Header_t          MessageHeader;
   unsigned long                  UpdateMemberFlag;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;
} DEVM_Update_Local_Device_Properties_Request_t;

#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_REQUEST_SIZE       (sizeof(DEVM_Update_Local_Device_Properties_Request_t))

   /* The following bit definitions are used with the UpdateMemberFlag  */
   /* member of the DEVM_Update_Local_Device_Properties_Request_t       */
   /* structure to denote which Device Property is to be updated.       */
#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CLASS_OF_DEVICE    0x00000001
#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DEVICE_NAME        0x00000002
#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DISCOVERABLE_MODE  0x00000003
#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CONNECTABLE_MODE   0x00000004
#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_PAIRABLE_MODE      0x00000005
#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DEVICE_APPEARANCE  0x00000006

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Update the Local Device Properties      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UPDATE_LOCAL_DEVICE_PROPERTIES  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Update_Local_Device_Properties_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Update_Local_Device_Properties_Response_t;

#define DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_RESPONSE_SIZE      (sizeof(DEVM_Update_Local_Device_Properties_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Local Device ID Information   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_QUERY_LOCAL_DEVICE_ID_INFORMATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Local_Device_ID_Information_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Query_Local_Device_ID_Information_Request_t;

#define DEVM_QUERY_LOCAL_DEVICE_ID_INFORMATION_REQUEST_SIZE    (sizeof(DEVM_Query_Local_Device_ID_Information_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Local Device ID Information   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_QUERY_LOCAL_DEVICE_ID_INFORMATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Local_Device_ID_Information_Response_t
{
   BTPM_Message_Header_t        MessageHeader;
   int                          Status;
   DEVM_Device_ID_Information_t LocalDeviceIDInformation;
} DEVM_Query_Local_Device_ID_Information_Response_t;

#define DEVM_QUERY_LOCAL_DEVICE_ID_INFORMATION_RESPONSE_SIZE   (sizeof(DEVM_Query_Local_Device_ID_Information_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add Internal EIR Data (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_ADD_INTERNAL_EIR_DATA            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Internal_EIR_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EIRDataLength;
   Byte_t                EIRData[1];
} DEVM_Add_Internal_EIR_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Add Internal EIR Data Request*/
   /* Message given the number of bytes present in the EIR data.  This  */
   /* function accepts as it's input the total number bytes that are    */
   /* present starting from the EIRData member of the                   */
   /* DEVM_Add_Internal_EIR_Data_Request_t structure and returns the    */
   /* total number of bytes required to hold the entire message.        */
#define DEVM_ADD_INTERNAL_EIR_DATA_REQUEST_SIZE(_x)            (STRUCTURE_OFFSET(DEVM_Add_Internal_EIR_Data_Request_t, EIRData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add Internal EIR Data (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_ADD_INTERNAL_EIR_DATA            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Internal_EIR_Data_Response_t
{
   BTPM_Message_Header_t        MessageHeader;
   int                          Status;
} DEVM_Add_Internal_EIR_Data_Response_t;

#define DEVM_ADD_INTERNAL_EIR_DATA_RESPONSE_SIZE               (sizeof(DEVM_Add_Internal_EIR_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add Extended Inquiry Response Data      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADD_LOCAL_EIR_DATA              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Local_EIR_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EIRDataLength;
   Byte_t                EIRData[1];
} DEVM_Add_Local_EIR_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Add Extended Inquiry Response*/
   /* Data Message given the number of bytes present in the Extended    */
   /* Inquiry Response data.  This function accepts as it's input the   */
   /* total number bytes that are present starting from the EIRData     */
   /* member of the DEVM_Add_Local_EIR_Data_Request_t structure and     */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define DEVM_ADD_LOCAL_EIR_DATA_REQUEST_SIZE(_x)               (STRUCTURE_OFFSET(DEVM_Add_Local_EIR_Data_Request_t, EIRData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add Extended Inquiry Response Data      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADD_LOCAL_EIR_DATA              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Local_EIR_Data_Response_t
{
   BTPM_Message_Header_t        MessageHeader;
   int                          Status;
} DEVM_Add_Local_EIR_Data_Response_t;

#define DEVM_ADD_LOCAL_EIR_DATA_RESPONSE_SIZE                  (sizeof(DEVM_Add_Local_EIR_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Remote Device Discovery (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_DEVICE_DISCOVERY          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Device_Discovery_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         DiscoveryDuration;
} DEVM_Start_Device_Discovery_Request_t;

#define DEVM_START_DEVICE_DISCOVERY_REQUEST_SIZE               (sizeof(DEVM_Start_Device_Discovery_Request_t))

   /* The following constants are used with the DiscoveryDuration member*/
   /* of the DEVM_Start_Device_Discovery_Request_t message to specify   */
   /* the Discovery Duration (in seconds).                              */
#define DEVM_START_DEVICE_DISCOVERY_DURATION_INDEFINITE        0x00000000

#define DEVM_START_DEVICE_DISCOVERY_DURATION_MINIMUM_SECONDS   0x00000001
#define DEVM_START_DEVICE_DISCOVERY_DURATION_MAXIMUM_SECONDS   0xFFFFFFFF

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Remote Device Discovery           */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_DEVICE_DISCOVERY          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Device_Discovery_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Start_Device_Discovery_Response_t;

#define DEVM_START_DEVICE_DISCOVERY_RESPONSE_SIZE              (sizeof(DEVM_Start_Device_Discovery_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Remote Device Discovery (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_DEVICE_DISCOVERY           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Device_Discovery_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Stop_Device_Discovery_Request_t;

#define DEVM_STOP_DEVICE_DISCOVERY_REQUEST_SIZE                (sizeof(DEVM_Stop_Device_Discovery_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Remote Device Discovery (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_DEVICE_DISCOVERY           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Device_Discovery_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Stop_Device_Discovery_Response_t;

#define DEVM_STOP_DEVICE_DISCOVERY_RESPONSE_SIZE               (sizeof(DEVM_Stop_Device_Discovery_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Remote Device Scan (Request).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_DEVICE_SCAN               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Device_Scan_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ScanDuration;
} DEVM_Start_Device_Scan_Request_t;

#define DEVM_START_DEVICE_SCAN_REQUEST_SIZE                    (sizeof(DEVM_Start_Device_Scan_Request_t))

   /* The following constants are used with the ScanDuration member of  */
   /* the DEVM_Start_Device_Scan_Request_t message to specify the Scan  */
   /* Duration (in seconds).                                            */
#define DEVM_START_DEVICE_SCAN_DURATION_INDEFINITE             0x00000000

#define DEVM_START_DEVICE_SCAN_DURATION_MINIMUM_SECONDS        0x00000001
#define DEVM_START_DEVICE_SCAN_DURATION_MAXIMUM_SECONDS        0xFFFFFFFF

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Remote Device Scan (Response).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_DEVICE_SCAN               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Device_Scan_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Start_Device_Scan_Response_t;

#define DEVM_START_DEVICE_SCAN_RESPONSE_SIZE                   (sizeof(DEVM_Start_Device_Scan_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Remote Device Scan (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_DEVICE_SCAN                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Device_Scan_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Stop_Device_Scan_Request_t;

#define DEVM_STOP_DEVICE_SCAN_REQUEST_SIZE                     (sizeof(DEVM_Stop_Device_Scan_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Remote Device Scan (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_DEVICE_SCAN                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Device_Scan_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Stop_Device_Scan_Response_t;

#define DEVM_STOP_DEVICE_SCAN_RESPONSE_SIZE                    (sizeof(DEVM_Stop_Device_Scan_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Advertising (Request).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_ADVERTISING               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Advertising_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         AdvertisingFlags;
   unsigned long         AdvertisingDuration;
   unsigned int          AdvertisingDataLength;
   Byte_t                AdvertisingData[1];
} DEVM_Start_Advertising_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Start Advertising Request    */
   /* Message given the number of bytes present in the advertising data.*/
   /* This function accepts as it's input the total number bytes that   */
   /* are present starting from the AdvertisingData member of the       */
   /* DEVM_Start_Advertising_Request_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
#define DEVM_START_ADVERTISING_REQUEST_SIZE(_x)                (STRUCTURE_OFFSET(DEVM_Start_Advertising_Request_t, AdvertisingData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Advertising (Response).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_ADVERTISING               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Advertising_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Start_Advertising_Response_t;

#define DEVM_START_ADVERTISING_RESPONSE_SIZE                   (sizeof(DEVM_Start_Advertising_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Advertising (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_ADVERTISING                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Advertising_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         AdvertisingFlags;
} DEVM_Stop_Advertising_Request_t;

#define DEVM_STOP_ADVERTISING_REQUEST_SIZE                     (sizeof(DEVM_Stop_Advertising_Request_t))

   /* The following constants are used with the StopAdvertisingFlags    */
   /* member of the DEVM_Stop_Advertising_Request_t message to specify  */
   /* various flags to apply to the advertising process.                */
#define DEVM_STOP_ADVERTISING_FLAGS_FORCE                      0x00000001

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Advertising (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_ADVERTISING                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Advertising_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Stop_Advertising_Response_t;

#define DEVM_STOP_ADVERTISING_RESPONSE_SIZE                    (sizeof(DEVM_Stop_Advertising_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Observation Scan (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_OBSERVATION_SCAN          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Observation_Scan_Request_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned long                 ObservationScanFlags;
   DEVM_Observation_Parameters_t ObservationParameters;
} DEVM_Start_Observation_Scan_Request_t;

#define DEVM_START_OBSERVATION_SCAN_REQUEST_SIZE                     (sizeof(DEVM_Start_Observation_Scan_Request_t))

   /* The following constants are used with the ObservationScanFlags    */
   /* member of the DEVM_Start_Observation_Scan_Request_t message to    */
   /* specify various flags to apply to the observation mode.           */
#define DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY                       0x00000001

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Start Observation Scan (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_START_OBSERVATION_SCAN          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Start_Observation_Scan_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Start_Observation_Scan_Response_t;

#define DEVM_START_OBSERVATION_SCAN_RESPONSE_SIZE                    (sizeof(DEVM_Start_Observation_Scan_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Observation Scan (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_OBSERVATION_SCAN           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Observation_Scan_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ObservationScanFlags;
} DEVM_Stop_Observation_Scan_Request_t;

#define DEVM_STOP_OBSERVATION_SCAN_REQUEST_SIZE                      (sizeof(DEVM_Stop_Observation_Scan_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Stop Observation Scan (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_STOP_OBSERVATION_SCAN           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Stop_Observation_Scan_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Stop_Observation_Scan_Response_t;

#define DEVM_STOP_OBSERVATION_SCAN_RESPONSE_SIZE                     (sizeof(DEVM_Stop_Observation_Scan_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to schedule an interleaved advertisement   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*        DEVM_MESSAGE_FUNCTION_SCHEDULE_INTERLEAVED_ADVERTISEMENT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Schedule_Interleaved_Advertisement_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DeviceManagerCallbackID;
   unsigned long         Flags;
   unsigned int          Duration;
   BD_ADDR_t             RandomAddress;
   unsigned int          DataLength;
   Byte_t                Data[1];
} DEVM_Schedule_Interleaved_Advertisement_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Local Device Manager Query   */
   /* Remote Device List Response Message given the number of Remote    */
   /* Device's present in the list (NOT the number of bytes occupied by */
   /* the list).  This function accepts as it's input the total number  */
   /* individual Remote Devices that are present starting from the Data */
   /* member of the DEVM_Schedule_Interleaved_Advertisement_Request_t   */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define DEVM_SCHEDULE_INTERLEAVED_ADVERTISEMENT_REQUEST_SIZE(_x)     (STRUCTURE_OFFSET(DEVM_Schedule_Interleaved_Advertisement_Request_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to schedule an interleaved advertisement   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*        DEVM_MESSAGE_FUNCTION_SCHEDULE_INTERLEAVED_ADVERTISEMENT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Schedule_Interleaved_Advertisement_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          InterleavedAdvertisementID;
} DEVM_Schedule_Interleaved_Advertisement_Response_t;

#define DEVM_SCHEDULE_INTERLEAVED_ADVERTISEMENT_RESPONSE_SIZE        (sizeof(DEVM_Schedule_Interleaved_Advertisement_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to cancel a previously scheduled           */
   /* interleaved advertisement (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           DEVM_MESSAGE_FUNCTION_CANCEL_INTERLEAVED_ADVERTISEMENT  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Cancel_Interleaved_Advertisement_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DeviceManagerCallbackID;
   unsigned int          InterleavedAdvertisementID;
} DEVM_Cancel_Interleaved_Advertisement_Request_t;

#define DEVM_CANCEL_INTERLEAVED_ADVERTISEMENT_REQUEST_SIZE           (sizeof(DEVM_Cancel_Interleaved_Advertisement_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to cancel a previously scheduled           */
   /* interleaved advertisement (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           DEVM_MESSAGE_FUNCTION_CANCEL_INTERLEAVED_ADVERTISEMENT  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Cancel_Interleaved_Advertisement_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Cancel_Interleaved_Advertisement_Response_t;

#define DEVM_CANCEL_INTERLEAVED_ADVERTISEMENT_RESPONSE_SIZE          (sizeof(DEVM_Cancel_Interleaved_Advertisement_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to suspend the scheduling of interleaved   */
   /* advertisements (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         DEVM_MESSAGE_FUNCTION_SUSPEND_INTERLEAVED_ADVERTISEMENTS  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Suspend_Interleaved_Advertisements_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Suspend_Interleaved_Advertisements_Request_t;

#define DEVM_SUSPEND_INTERLEAVED_ADVERTISEMENTS_REQUEST_SIZE                      (sizeof(DEVM_Suspend_Interleaved_Advertisements_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to suspend the scheduling of interleaved   */
   /* advertisements (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         DEVM_MESSAGE_FUNCTION_SUSPEND_INTERLEAVED_ADVERTISEMENTS  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Suspend_Interleaved_Advertisements_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Suspend_Interleaved_Advertisements_Response_t;

#define DEVM_SUSPEND_INTERLEAVED_ADVERTISEMENTS_RESPONSE_SIZE                     (sizeof(DEVM_Suspend_Interleaved_Advertisements_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to resume the scheduling of interleaved    */
   /* advertisements (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         DEVM_MESSAGE_FUNCTION_RESUME_INTERLEAVED_ADVERTISEMENTS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Resume_Interleaved_Advertisements_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Resume_Interleaved_Advertisements_Request_t;

#define DEVM_RESUME_INTERLEAVED_ADVERTISEMENTS_REQUEST_SIZE          (sizeof(DEVM_Resume_Interleaved_Advertisements_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to resume the scheduling of interleaved    */
   /* advertisements (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         DEVM_MESSAGE_FUNCTION_RESUME_INTERLEAVED_ADVERTISEMENTS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Resume_Interleaved_Advertisements_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Resume_Interleaved_Advertisements_Response_t;

#define DEVM_RESUME_INTERLEAVED_ADVERTISEMENTS_RESPONSE_SIZE         (sizeof(DEVM_Resume_Interleaved_Advertisements_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device List (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_LIST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_List_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RemoteDeviceFilter;
   Class_of_Device_t     ClassOfDeviceFilter;
} DEVM_Query_Remote_Device_List_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_LIST_REQUEST_SIZE             (sizeof(DEVM_Query_Remote_Device_List_Request_t))

   /* The following constants are used with the RemoteDeviceFilter      */
   /* member of the DEVM_Query_Remote_Device_List_Request_t message to  */
   /* specify the type of Devices to return (e.g. a filter to apply).   */
   /* * NOTE * Bit masks are provided to be able to further filter the  */
   /*          devices based on device type.                            */
#define DEVM_QUERY_REMOTE_DEVICE_LIST_ALL_DEVICES              0x00000000
#define DEVM_QUERY_REMOTE_DEVICE_LIST_CURRENTLY_CONNECTED      0x00000001
#define DEVM_QUERY_REMOTE_DEVICE_LIST_CURRENTLY_PAIRED         0x00000002
#define DEVM_QUERY_REMOTE_DEVICE_LIST_CURRENTLY_UNPAIRED       0x00000003

#define DEVM_QUERY_REMOTE_DEVICE_LIST_NO_LOW_ENERGY_DEVICES    0x40000000
#define DEVM_QUERY_REMOTE_DEVICE_LIST_NO_BR_EDR_DEVICES        0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device List (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_LIST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_List_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberRemoteDevices;
   BD_ADDR_t             RemoteDeviceAddressList[1];
} DEVM_Query_Remote_Device_List_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Local Device Manager Query   */
   /* Remote Device List Response Message given the number of Remote    */
   /* Device's present in the list (NOT the number of bytes occupied by */
   /* the list).  This function accepts as it's input the total number  */
   /* individual Remote Devices that are present starting from the      */
   /* RemoteDeviceAddressList member of the                             */
   /* DEVM_Query_Remote_Device_List_Response_t structure and returns the*/
   /* total number of bytes required to hold the entire message.        */
#define DEVM_QUERY_REMOTE_DEVICE_LIST_RESPONSE_SIZE(_x)        (STRUCTURE_OFFSET(DEVM_Query_Remote_Device_List_Response_t, RemoteDeviceAddressList) + (((unsigned int)(_x))*(BD_ADDR_SIZE)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Properties      */
   /* of an individual Remote Device (Request).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_PROPERTIES  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Properties_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         QueryFlags;
} DEVM_Query_Remote_Device_Properties_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_REQUEST_SIZE       (sizeof(DEVM_Query_Remote_Device_Properties_Request_t))

   /* The following constants represent the bit mask values that can be */
   /* used to specify various modifier flags for the query remote device*/
   /* properties operation.                                             */
#define DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_FORCE_UPDATE             0x00000001
#define DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_UPDATE_CONNECTION_RSSI   0x00000002
#define DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY               0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Properties      */
   /* of an individual Remote Device (Response).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_PROPERTIES  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Properties_Response_t
{
   BTPM_Message_Header_t           MessageHeader;
   int                             Status;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Query_Remote_Device_Properties_Response_t;

#define DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_RESPONSE_SIZE      (sizeof(DEVM_Query_Remote_Device_Properties_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Services of an  */
   /* individual Remote Device (Request).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICES    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Services_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         QueryFlags;
} DEVM_Query_Remote_Device_Services_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_SERVICES_REQUEST_SIZE         (sizeof(DEVM_Query_Remote_Device_Services_Request_t))

   /* The following constants represent the bit mask values that can be */
   /* used to specify various modifier flags for the query remote device*/
   /* services operation.                                               */
#define DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE      0x00000001
#define DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY        0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Services of an  */
   /* individual Remote Device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICES    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Services_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ServiceDataLength;
   unsigned char         ServiceData[1];
} DEVM_Query_Remote_Device_Services_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Local Device Manager Query   */
   /* Remote Device Services Response Message given the number of       */
   /* individual Service Data bytes present in the ServiceData member of*/
   /* the message.  This function accepts as it's input the total number*/
   /* individual ServiceData bytes that are present starting from the   */
   /* ServiceData member of the                                         */
   /* DEVM_Query_Remote_Device_Services_Response_t structure and returns*/
   /* the total number of bytes required to hold the entire message.    */
#define DEVM_QUERY_REMOTE_DEVICE_SERVICES_RESPONSE_SIZE(_x)    (STRUCTURE_OFFSET(DEVM_Query_Remote_Device_Services_Response_t, ServiceData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query whether a Remote Device supports a*/
   /* specified Service.                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICE_SUPPORTED */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Service_Supported_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   SDP_UUID_Entry_t      ServiceUUID;
} DEVM_Query_Remote_Device_Service_Supported_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_SERVICE_SUPPORTED_REQUEST_SIZE   (sizeof(DEVM_Query_Remote_Device_Service_Supported_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Services of an  */
   /* individual Remote Device (Response).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICES    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Service_Supported_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Query_Remote_Device_Service_Supported_Response_t;

#define DEVM_QUERY_REMOTE_DEVICE_SERVICE_SUPPORTED_RESPONSE_SIZE  (sizeof(DEVM_Query_Remote_Device_Service_Supported_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query which known Remote Devices support*/
   /* a particular Service (Request).                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICES_FOR_SERVICE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Devices_For_Service_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   SDP_UUID_Entry_t      ServiceUUID;
} DEVM_Query_Remote_Devices_For_Service_Request_t;

#define DEVM_QUERY_REMOTE_DEVICES_FOR_SERVICE_REQUEST_SIZE     (sizeof(DEVM_Query_Remote_Devices_For_Service_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query which known Remote Devices support*/
   /* a particular Service (Response).                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICES_FOR_SERVICE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Devices_For_Service_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberRemoteDevices;
   BD_ADDR_t             RemoteDeviceAddressList[1];
} DEVM_Query_Remote_Devices_For_Service_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold a an entire Local Device Manager    */
   /* Query Remote Devices For Service Response Message given the       */
   /* number of Remote Device's present in the list (NOT the number     */
   /* of bytes occupied by the list). This function accepts as it's     */
   /* input the total number individual Remote Devices that are         */
   /* present starting from the RemoteDeviceAddressList member of the   */
   /* DEVM_Query_Remote_Devices_For_Service_Response_t structure and    */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define DEVM_QUERY_REMOTE_DEVICES_FOR_SERVICE_RESPONSE_SIZE(_x)   (STRUCTURE_OFFSET(DEVM_Query_Remote_Devices_For_Service_Response_t, RemoteDeviceAddressList) + (((unsigned int)(_x))*(BD_ADDR_SIZE)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Service Classes */
   /* of an individual Remote Device (Request).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICE_CLASSES */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Service_Classes_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Query_Remote_Device_Service_Classes_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_SERVICE_CLASSES_REQUEST_SIZE  (sizeof(DEVM_Query_Remote_Device_Service_Classes_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Service Classes */
   /* of an individual Remote Device (Response).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_SERVICE_CLASSES */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Service_Classes_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberServiceClasses;
   SDP_UUID_Entry_t      ServiceClassList[1];
} DEVM_Query_Remote_Device_Service_Classes_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold a an entire Local Device Manager    */
   /* Query Remote Device Service Classes Response Message given the    */
   /* number of Service Classes UUIDs present in the list (NOT the      */
   /* number of bytes occupied by the list). This function accepts      */
   /* as it's input the total number individual Service Classes that    */
   /* are present starting from the ServiceClassList member of the      */
   /* DEVM_Query_Remote_Device_Service_Classes_Response_t structure and */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define DEVM_QUERY_REMOTE_DEVICE_SERVICE_CLASSES_RESPONSE_SIZE(_x)   (STRUCTURE_OFFSET(DEVM_Query_Remote_Device_Service_Classes_Response_t, ServiceClassList) + (((unsigned int)(_x))*(SDP_UUID_ENTRY_SIZE)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Extended Inquiry*/
   /* (EIR) Data of an individual Remote Device (Request).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_EIR_DATA    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_EIR_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Query_Remote_Device_EIR_Data_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_EIR_DATA_REQUEST_SIZE         (sizeof(DEVM_Query_Remote_Device_EIR_Data_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Extended Inquiry*/
   /* (EIR) Data of an individual Remote Device (Response).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_EIR_DATA    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_EIR_Data_Response_t
{
   BTPM_Message_Header_t            MessageHeader;
   int                              Status;
   Boolean_t                        EIRDataValid;
   Extended_Inquiry_Response_Data_t EIRData;
} DEVM_Query_Remote_Device_EIR_Data_Response_t;

#define DEVM_QUERY_REMOTE_DEVICE_EIR_DATA_RESPONSE_SIZE        (sizeof(DEVM_Query_Remote_Device_EIR_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Advertising/Scan*/
   /* Response Data of an individual Remote Device (Request).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*        DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ADVERTISING_DATA */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Advertising_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Query_Remote_Device_Advertising_Data_Request_t;

#define DEVM_QUERY_REMOTE_DEVICE_ADVERTISING_DATA_REQUEST_SIZE (sizeof(DEVM_Query_Remote_Device_Advertising_Data_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Query the Remote Device Advertising/Scan*/
   /* Response Data of an individual Remote Device (Response).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*        DEVM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ADVERTISING_DATA */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Remote_Device_Advertising_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   Boolean_t             AdvertisingDataValid;
   Boolean_t             ScanResponseDataValid;
   Advertising_Data_t    AdvertisingData;
   Advertising_Data_t    ScanResponseData;
} DEVM_Query_Remote_Device_Advertising_Data_Response_t;

#define DEVM_QUERY_REMOTE_DEVICE_ADVERTISING_DATA_RESPONSE_SIZE   (sizeof(DEVM_Query_Remote_Device_Advertising_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add the specified Remote Device to the  */
   /* Local Device Manager's Remote Device database (Request).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADD_REMOTE_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Remote_Device_Request_t
{
   BTPM_Message_Header_t                 MessageHeader;
   BD_ADDR_t                             RemoteDeviceAddress;
   Class_of_Device_t                     ClassOfDevice;
   unsigned long                         AddRemoteDeviceFlags;
   DEVM_Remote_Device_Application_Data_t ApplicationData;
} DEVM_Add_Remote_Device_Request_t;

#define DEVM_ADD_REMOTE_DEVICE_REQUEST_SIZE                    (sizeof(DEVM_Add_Remote_Device_Request_t))

   /* The following constants are used with the AddRemoteDeviceFlags    */
   /* member of the DEVM_Add_Remote_Device_Request_t message to specify */
   /* various flags to apply to the device and/or data.                 */
#define DEVM_ADD_REMOTE_DEVICE_FLAGS_APPLICATION_DATA_VALID    0x00000001
#define DEVM_ADD_REMOTE_DEVICE_FLAGS_LOW_ENERGY_DEVICE         0x40000000
#define DEVM_ADD_REMOTE_DEVICE_FLAGS_BR_EDR_DEVICE             0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add the specified Remote Device to the  */
   /* Local Device Manager's Remote Device database (Response).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADD_REMOTE_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Add_Remote_Device_Response_t;

#define DEVM_ADD_REMOTE_DEVICE_RESPONSE_SIZE                   (sizeof(DEVM_Add_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete the specified Remote Device from */
   /* the Local Device Manager's Remote Device database (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Delete_Remote_Device_Request_t;

#define DEVM_DELETE_REMOTE_DEVICE_REQUEST_SIZE                 (sizeof(DEVM_Delete_Remote_Device_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete the specified Remote Device from */
   /* the Local Device Manager's Remote Device database (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Delete_Remote_Device_Response_t;

#define DEVM_DELETE_REMOTE_DEVICE_RESPONSE_SIZE                (sizeof(DEVM_Delete_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Update the specified Remote Device's    */
   /* Application Data in the Local Device Manager's Remote Device      */
   /* database (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UPDATE_REMOTE_DEVICE_APP_DATA   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Update_Remote_Device_Application_Data_Request_t
{
   BTPM_Message_Header_t                 MessageHeader;
   BD_ADDR_t                             RemoteDeviceAddress;
   Boolean_t                             ApplicationDataValid;
   DEVM_Remote_Device_Application_Data_t ApplicationData;
} DEVM_Update_Remote_Device_Application_Data_Request_t;

#define DEVM_UPDATE_REMOTE_DEVICE_APPLICATION_DATA_REQUEST_SIZE   (sizeof(DEVM_Update_Remote_Device_Application_Data_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Update the specified Remote Device's    */
   /* Application Data in the Local Device Manager's Remote Device      */
   /* database (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UPDATE_REMOTE_DEVICE_APP_DATA   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Update_Remote_Device_Application_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Update_Remote_Device_Application_Data_Response_t;

#define DEVM_UPDATE_REMOTE_DEVICE_APPLICATION_DATA_RESPONSE_SIZE  (sizeof(DEVM_Update_Remote_Device_Application_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete the specified Remote Devices from*/
   /* the Local Device Manager's Remote Device database (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_REMOTE_DEVICES           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Remote_Devices_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DeleteDevicesFilter;
} DEVM_Delete_Remote_Devices_Request_t;

#define DEVM_DELETE_REMOTE_DEVICES_REQUEST_SIZE                (sizeof(DEVM_Delete_Remote_Devices_Request_t))

   /* The following constants are used with the DeleteDevicesFilter     */
   /* member of the DEVM_Delete_Remote_Devices_Request_t message to     */
   /* specify the types of Devices to delete (e.g. a filter to apply).  */
   /* * NOTE * Bit masks are provided to further filter the deletion    */
   /*          by device type.                                          */
#define DEVM_DELETE_REMOTE_DEVICES_ALL_DEVICES                 0x00000000
#define DEVM_DELETE_REMOTE_DEVICES_PAIRED_DEVICES              0x00000001
#define DEVM_DELETE_REMOTE_DEVICES_UNPAIRED_DEVICES            0x00000002
#define DEVM_DELETE_REMOTE_DEVICES_LOW_ENERGY_DEVICES_ONLY     0x40000000
#define DEVM_DELETE_REMOTE_DEVICES_BR_EDR_DEVICES_ONLY         0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete the specified Remote Devices from*/
   /* the Local Device Manager's Remote Device database (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_REMOTE_DEVICES           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Remote_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Delete_Remote_Devices_Response_t;

#define DEVM_DELETE_REMOTE_DEVICES_RESPONSE_SIZE               (sizeof(DEVM_Delete_Remote_Devices_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Pair with the specified Remote Device   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_PAIR_WITH_REMOTE_DEVICE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Pair_With_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         PairFlags;
} DEVM_Pair_With_Remote_Device_Request_t;

#define DEVM_PAIR_WITH_REMOTE_DEVICE_REQUEST_SIZE              (sizeof(DEVM_Pair_With_Remote_Device_Request_t))

   /* The following constants represent the bit mask flags that are used*/
   /* with the PairFlags member of the                                  */
   /* DEVM_Pair_With_Remote_Device_Request_t message to specify various */
   /* modifiers to the pairing process.                                 */
#define DEVM_PAIR_WITH_REMOTE_DEVICE_FLAGS_FORCE_PAIR          0x00000001
#define DEVM_PAIR_WITH_REMOTE_DEVICE_FLAGS_KEEP_CONNECTION     0x00000002
#define DEVM_PAIR_WITH_REMOTE_DEVICE_FLAGS_LOW_ENERGY          0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Pair with the specified Remote Device   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_PAIR_WITH_REMOTE_DEVICE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Pair_With_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Pair_With_Remote_Device_Response_t;

#define DEVM_PAIR_WITH_REMOTE_DEVICE_RESPONSE_SIZE             (sizeof(DEVM_Pair_With_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Cancel a current Pairing with the       */
   /* specified Remote Device (Request).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_CANCEL_PAIR_WITH_REMOTE_DEVICE  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Cancel_Pair_With_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Cancel_Pair_With_Remote_Device_Request_t;

#define DEVM_CANCEL_PAIR_WITH_REMOTE_DEVICE_REQUEST_SIZE       (sizeof(DEVM_Cancel_Pair_With_Remote_Device_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Cancel a current Pairing with the       */
   /* specified Remote Device (Response).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_CANCEL_PAIR_WITH_REMOTE_DEVICE  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Cancel_Pair_With_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Cancel_Pair_With_Remote_Device_Response_t;

#define DEVM_CANCEL_PAIR_WITH_REMOTE_DEVICE_RESPONSE_SIZE      (sizeof(DEVM_Cancel_Pair_With_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Un-Pair with the specified Remote Device*/
   /* (Request).                                                        */
   /* * NOTE * This is a legacy message format.  It exists only to      */
   /*          maintain compatibility with older clients.               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UNPAIR_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_UnPair_Remote_Device_Legacy_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_UnPair_Remote_Device_Legacy_Request_t;

#define DEVM_UNPAIR_REMOTE_DEVICE_LEGACY_REQUEST_SIZE          (sizeof(DEVM_UnPair_Remote_Device_Legacy_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Un-Pair with the specified Remote Device*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UNPAIR_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_UnPair_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         UnPairFlags;
} DEVM_UnPair_Remote_Device_Request_t;

#define DEVM_UNPAIR_REMOTE_DEVICE_REQUEST_SIZE                 (sizeof(DEVM_UnPair_Remote_Device_Request_t))

   /* The following bit definitions are used with the UnPairFlags member*/
   /* of the DEVM_UnPair_Remote_Device_Request_t structure to denote the*/
   /* type of un-pair operation to perform.                             */
#define DEVM_UNPAIR_REMOTE_DEVICE_FLAGS_LOW_ENERGY             0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Un-Pair with the specified Remote Device*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UNPAIR_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_UnPair_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_UnPair_Remote_Device_Response_t;

#define DEVM_UNPAIR_REMOTE_DEVICE_RESPONSE_SIZE                (sizeof(DEVM_UnPair_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Authenticate with the specified Remote  */
   /* Device (Request).                                                 */
   /* * NOTE * This is a legacy message format.  It exists only to      */
   /*          maintain compatibility with older clients.               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_AUTHENTICATE_REMOTE_DEVICE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Authenticate_Remote_Device_Legacy_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Authenticate_Remote_Device_Legacy_Request_t;

#define DEVM_AUTHENTICATE_REMOTE_DEVICE_LEGACY_REQUEST_SIZE    (sizeof(DEVM_Authenticate_Remote_Device_Legacy_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Authenticate with the specified Remote  */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_AUTHENTICATE_REMOTE_DEVICE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Authenticate_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         AuthenticateFlags;
} DEVM_Authenticate_Remote_Device_Request_t;

#define DEVM_AUTHENTICATE_REMOTE_DEVICE_REQUEST_SIZE           (sizeof(DEVM_Authenticate_Remote_Device_Request_t))

   /* The following bit definitions are used with the AuthenticateFlags */
   /* member of the DEVM_Authenticate_Remote_Device_Request_t structure */
   /* to denote the type of authentication operation to perform.        */
#define DEVM_AUTHENTICATE_REMOTE_DEVICE_FLAGS_LOW_ENERGY       0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Authenticate with the specified Remote  */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_AUTHENTICATE_REMOTE_DEVICE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Authenticate_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Authenticate_Remote_Device_Response_t;

#define DEVM_AUTHENTICATE_REMOTE_DEVICE_RESPONSE_SIZE          (sizeof(DEVM_Authenticate_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Encrypt the specified Remote Device     */
   /* (Request).                                                        */
   /* * NOTE * This is a legacy message format.  It exists only to      */
   /*          maintain compatibility with older clients.               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENCRYPT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Encrypt_Remote_Device_Legacy_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Encrypt_Remote_Device_Legacy_Request_t;

#define DEVM_ENCRYPT_REMOTE_DEVICE_LEGACY_REQUEST_SIZE         (sizeof(DEVM_Encrypt_Remote_Device_Legacy_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Encrypt the specified Remote Device     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENCRYPT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Encrypt_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         EncryptFlags;
} DEVM_Encrypt_Remote_Device_Request_t;

#define DEVM_ENCRYPT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(DEVM_Encrypt_Remote_Device_Request_t))

   /* The following bit definitions are used with the EncryptFlags      */
   /* member of the DEVM_Encrypt_Remote_Device_Request_t structure to   */
   /* denote the type of encryption operation to perform.               */
#define DEVM_ENCRYPT_REMOTE_DEVICE_FLAGS_LOW_ENERGY            0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Encrypt the specified Remote Device     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENCRYPT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Encrypt_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Encrypt_Remote_Device_Response_t;

#define DEVM_ENCRYPT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(DEVM_Encrypt_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Connect to the specified Remote Device  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_CONNECT_WITH_REMOTE_DEVICE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Connect_With_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         ConnectFlags;
} DEVM_Connect_With_Remote_Device_Request_t;

#define DEVM_CONNECT_WITH_REMOTE_DEVICE_REQUEST_SIZE           (sizeof(DEVM_Connect_With_Remote_Device_Request_t))

   /* The following constants are used with the ConnectFlags member of  */
   /* the DEVM_Connect_With_Remote_Device_Request_t message to specify  */
   /* various flags to apply to the connection process.                 */
#define DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE        0x00000001
#define DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT             0x00000002
#define DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_USE_RANDOM_ADDRESS  0x00000004
#define DEVM_CONNECT_WITH_REMOTE_DEVICE_FORCE_LOW_ENERGY          0x80000000
#define DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_EXTENDED_PACKETS    0x40000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Connect to the specified Remote Device  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_CONNECT_WITH_REMOTE_DEVICE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Connect_With_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Connect_With_Remote_Device_Response_t;

#define DEVM_CONNECT_WITH_REMOTE_DEVICE_RESPONSE_SIZE          (sizeof(DEVM_Connect_With_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Disconnect from the specified Remote    */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Disconnect_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         DisconnectFlags;
} DEVM_Disconnect_Remote_Device_Request_t;

#define DEVM_DISCONNECT_REMOTE_DEVICE_REQUEST_SIZE             (sizeof(DEVM_Disconnect_Remote_Device_Request_t))

   /* The following constants are used with the DisconnectFlags member  */
   /* of the DEVM_Disconnect_Remote_Device_Request_t message to specify */
   /* various flags to apply to the disconnection process.              */
#define DEVM_DISCONNECT_FROM_REMOTE_DEVICE_FLAGS_FORCE         0x00000001
#define DEVM_DISCONNECT_FROM_REMOTE_DEVICE_FLAGS_LOW_ENERGY    0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Disconnect from the specified Remote    */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Disconnect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Disconnect_Remote_Device_Response_t;

#define DEVM_DISCONNECT_REMOTE_DEVICE_RESPONSE_SIZE            (sizeof(DEVM_Disconnect_Remote_Device_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to set the current link mode of a specified*/
   /* Remote Device to active (Request).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_SET_REMOTE_DEVICE_LINK_ACTIVE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Set_Remote_Device_Link_Mode_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Set_Remote_Device_Link_Mode_Request_t;

#define DEVM_SET_REMOTE_DEVICE_LINK_MODE_REQUEST_SIZE          (sizeof(DEVM_Set_Remote_Device_Link_Mode_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to set the current link mode of a specified*/
   /* Remote Device to active (Response).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_SET_REMOTE_DEVICE_LINK_ACTIVE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Set_Remote_Device_Link_Mode_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Set_Remote_Device_Link_Mode_Response_t;

#define DEVM_SET_REMOTE_DEVICE_LINK_MODE_RESPONSE_SIZE         (sizeof(DEVM_Set_Remote_Device_Link_Mode_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Create a Service Record in the Local SDP*/
   /* Database (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_CREATE_SERVICE_RECORD           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Create_Service_Record_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   Boolean_t             PersistClient;
   unsigned int          NumberServiceClassUUID;
   SDP_UUID_Entry_t      SDP_UUID_Entry[1];
} DEVM_Create_Service_Record_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Local Device Manager Create  */
   /* Service Record Request Message given the number of SDP UUID       */
   /* Entries present in the list (NOT the number of bytes occupied by  */
   /* the list).  This function accepts as it's input the total number  */
   /* individual SDP UUID Entries that are present starting from the    */
   /* SDP_UUID_Entry member of the DEVM_Create_Service_Record_Request_t */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define DEVM_CREATE_SERVICE_RECORD_REQUEST_SIZE(_x)               (STRUCTURE_OFFSET(DEVM_Create_Service_Record_Request_t, SDP_UUID_Entry) + (((unsigned int)(_x))*(SDP_UUID_ENTRY_SIZE)))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Create a Service Record in the Local SDP*/
   /* Database (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_CREATE_SERVICE_RECORD           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Create_Service_Record_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned long         ServiceRecordHandle;
} DEVM_Create_Service_Record_Response_t;

#define DEVM_CREATE_SERVICE_RECORD_RESPONSE_SIZE               (sizeof(DEVM_Create_Service_Record_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete a Service Record in the Local SDP*/
   /* Database (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_SERVICE_RECORD           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Service_Record_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ServiceRecordHandle;
} DEVM_Delete_Service_Record_Request_t;

#define DEVM_DELETE_SERVICE_RECORD_REQUEST_SIZE                (sizeof(DEVM_Delete_Service_Record_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete a Service Record in the Local SDP*/
   /* Database (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_SERVICE_RECORD           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Service_Record_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Delete_Service_Record_Response_t;

#define DEVM_DELETE_SERVICE_RECORD_RESPONSE_SIZE               (sizeof(DEVM_Delete_Service_Record_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add a Service Record Attribute to a     */
   /* Service Record on the Local Device (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADD_SERVICE_RECORD_ATTRIBUTE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Service_Record_Attribute_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ServiceRecordHandle;
   unsigned int          AttributeID;
   unsigned int          AttributeDataLength;
   unsigned char         AttributeData[1];
} DEVM_Add_Service_Record_Attribute_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Local Device Manager Add     */
   /* Service Record Attribute Request Message given the number of      */
   /* individual Attribute Data bytes present in the AttributeData      */
   /* member of the message.  This function accepts as it's input the   */
   /* total number individual Attribute Data bytes that are present     */
   /* starting from the AttributeData member of the                     */
   /* DEVM_Add_Service_Record_Attribute_Request_t structure and returns */
   /* the total number of bytes required to hold the entire message.    */
#define DEVM_ADD_SERVICE_RECORD_ATTRIBUTE_REQUEST_SIZE(_x)     (STRUCTURE_OFFSET(DEVM_Add_Service_Record_Attribute_Request_t, AttributeData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Add a Service Record Attribute to a     */
   /* Service Record on the Local Device (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADD_SERVICE_RECORD_ATTRIBUTE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Add_Service_Record_Attribute_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Add_Service_Record_Attribute_Response_t;

#define DEVM_ADD_SERVICE_RECORD_ATTRIBUTE_RESPONSE_SIZE        (sizeof(DEVM_Add_Service_Record_Attribute_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete a Service Record Attribute from  */
   /* a Service Record on the Local Device (Request).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_SERVICE_RECORD_ATTRIBUTE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Service_Record_Attribute_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ServiceRecordHandle;
   unsigned int          AttributeID;
} DEVM_Delete_Service_Record_Attribute_Request_t;

#define DEVM_DELETE_SERVICE_RECORD_ATTRIBUTE_REQUEST_SIZE      (sizeof(DEVM_Add_Service_Record_Attribute_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Delete a Service Record Attribute from  */
   /* a Service Record on the Local Device (Response).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DELETE_SERVICE_RECORD_ATTRIBUTE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Delete_Service_Record_Attribute_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Delete_Service_Record_Attribute_Response_t;

#define DEVM_DELETE_SERVICE_RECORD_ATTRIBUTE_RESPONSE_SIZE     (sizeof(DEVM_Delete_Service_Record_Attribute_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to instruct the local Service to           */
   /* enable/disable Bluetooth Debugging (Request).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENABLE_BLUETOOTH_DEBUG          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Enable_Bluetooth_Debug_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   Boolean_t             Enable;
   unsigned int          DebugType;
   unsigned long         DebugFlags;
   unsigned int          DebugParameterLength;
   unsigned char         DebugParameter[1];
} DEVM_Enable_Bluetooth_Debug_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Device Manager Enable        */
   /* Bluetooth Debug Request Message given the total number of         */
   /* Parameter bytes that are present in the Message.  This function   */
   /* accepts as it's input the total number individual Parameter bytes */
   /* that are present in the request starting from the DebugParameter  */
   /* member of the DEVM_Enable_Bluetooth_Debug_Request_t structure and */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define DEVM_ENABLE_BLUETOOTH_DEBUG_REQUEST_SIZE(_x)           (STRUCTURE_OFFSET(DEVM_Enable_Bluetooth_Debug_Request_t, DebugParameter) + (unsigned int)(_x))

   /* The following constants are used with the DebugFlags member of the*/
   /* DEVM_Enable_Bluetooth_Debug_Request_t structure to denote the     */
   /* flags to apply to the Debugging that is being enabled.            */
#define DEVM_BLUETOOTH_DEBUG_FLAGS_APPEND_FILE                 0x00000001

   /* The following constants are used with the DebugType member of the */
   /* DEVM_Enable_Bluetooth_Debug_Request_t structure to denote the type*/
   /* of Debugging to Enable.                                           */
#define DEVM_BLUETOOTH_DEBUG_TYPE_ASCII_LOG_FILE               0x00000001
#define DEVM_BLUETOOTH_DEBUG_TYPE_TERMINAL                     0x00000002
#define DEVM_BLUETOOTH_DEBUG_TYPE_FTS_LOG_FILE                 0x00000003

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to instruct the local Service to           */
   /* enable/disable Bluetooth Debugging (Response).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENABLE_BLUETOOTH_DEBUG          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Enable_Bluetooth_Debug_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Enable_Bluetooth_Debug_Response_t;

#define DEVM_ENABLE_BLUETOOTH_DEBUG_RESPONSE_SIZE              (sizeof(DEVM_Enable_Bluetooth_Debug_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Register a specific Client to process   */
   /* Authentication messages (Request).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REGISTER_AUTHENTICATION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Register_Authentication_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Register_Authentication_Request_t;

#define DEVM_REGISTER_AUTHENTICATION_REQUEST_SIZE              (sizeof(DEVM_Register_Authentication_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Register a specific Client to process   */
   /* Authentication messages (Response).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REGISTER_AUTHENTICATION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Register_Authentication_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          AuthenticationHandlerID;
} DEVM_Register_Authentication_Response_t;

#define DEVM_REGISTER_AUTHENTICATION_RESPONSE_SIZE             (sizeof(DEVM_Register_Authentication_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Un-Register a previously registered     */
   /* Client from processing Authentication messages (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UNREGISTER_AUTHENTICATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_UnRegister_Authentication_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          AuthenticationHandlerID;
} DEVM_UnRegister_Authentication_Request_t;

#define DEVM_UNREGISTER_AUTHENTICATION_REQUEST_SIZE            (sizeof(DEVM_UnRegister_Authentication_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Un-Register a previously registered     */
   /* Client from processing Authentication messages (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_UNREGISTER_AUTHENTICATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_UnRegister_Authentication_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_UnRegister_Authentication_Response_t;

#define DEVM_UNREGISTER_AUTHENTICATION_RESPONSE_SIZE           (sizeof(DEVM_UnRegister_Authentication_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that represents an Authentication Response */
   /* from the Client when responding to an Authentication Request      */
   /* message (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_AUTHENTICATION_RESPONSE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Authentication_Response_Request_t
{
   BTPM_Message_Header_t             MessageHeader;
   unsigned int                      AuthenticationHandlerID;
   DEVM_Authentication_Information_t AuthenticationInformation;
} DEVM_Authentication_Response_Request_t;

#define DEVM_AUTHENTICATION_RESPONSE_REQUEST_SIZE              (sizeof(DEVM_Authentication_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that represents an Authentication Response */
   /* from the Client when responding to an Authentication Request      */
   /* message (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_AUTHENTICATION_RESPONSE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Authentication_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Authentication_Response_Response_t;

#define DEVM_AUTHENTICATION_RESPONSE_RESPONSE_SIZE             (sizeof(DEVM_Authentication_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to send a raw HCI command to the Bluetooth */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_SEND_RAW_HCI_COMMAND            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Send_Raw_HCI_Command_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned char         OGF;
   unsigned short        OCF;
   Boolean_t             WaitForResponse;
   unsigned int          CommandLength;
   unsigned char         CommandData[255];
} DEVM_Send_Raw_HCI_Command_Request_t;

#define DEVM_SEND_RAW_HCI_COMMAND_REQUEST_SIZE                 (sizeof(DEVM_Send_Raw_HCI_Command_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to send a raw HCI command to the Bluetooth */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_SEND_RAW_HCI_COMMAND            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Send_Raw_HCI_Command_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned char         ResultStatus;
   unsigned int          ResultLength;
   unsigned char         ResultData[255];
} DEVM_Send_Raw_HCI_Command_Response_t;

#define DEVM_SEND_RAW_HCI_COMMAND_RESPONSE_SIZE                (sizeof(DEVM_Send_Raw_HCI_Command_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to enable a specified Local Device Feature */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENABLE_LOCAL_DEVICE_FEATURE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Enable_Local_Device_Feature_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         LocalDeviceFeature;
} DEVM_Enable_Local_Device_Feature_Request_t;

#define DEVM_ENABLE_LOCAL_DEVICE_FEATURE_REQUEST_SIZE          (sizeof(DEVM_Enable_Local_Device_Feature_Request_t))

   /* The following constants represent the valid constants that may be */
   /* set in the LocalDeviceFeature member of the                       */
   /* DEVM_Enable_Local_Device_Feature_Request_t structure.             */
#define DEVM_LOCAL_DEVICE_FEATURE_BLUETOOTH_LOW_ENERGY         0x00000001
#define DEVM_LOCAL_DEVICE_FEATURE_ANT_PLUS                     0x00000002

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to enable a specified Local Device Feature */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ENABLE_LOCAL_DEVICE_FEATURE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Enable_Local_Device_Feature_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Enable_Local_Device_Feature_Response_t;

#define DEVM_ENABLE_LOCAL_DEVICE_FEATURE_RESPONSE_SIZE         (sizeof(DEVM_Enable_Local_Device_Feature_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Disable a specified Local Device Feature*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DISABLE_LOCAL_DEVICE_FEATURE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Disable_Local_Device_Feature_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         LocalDeviceFeature;
} DEVM_Disable_Local_Device_Feature_Request_t;

#define DEVM_DISABLE_LOCAL_DEVICE_FEATURE_REQUEST_SIZE          (sizeof(DEVM_Disable_Local_Device_Feature_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to Disable a specified Local Device Feature*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DISABLE_LOCAL_DEVICE_FEATURE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Disable_Local_Device_Feature_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Disable_Local_Device_Feature_Response_t;

#define DEVM_DISABLE_LOCAL_DEVICE_FEATURE_RESPONSE_SIZE         (sizeof(DEVM_Disable_Local_Device_Feature_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to query the Local Device Features that are*/
   /* currently active (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*          DEVM_MESSAGE_FUNCTION_QUERY_ACTIVE_LOCAL_DEVICE_FEATURES */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Active_Local_Device_Features_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Query_Active_Local_Device_Features_Request_t;

#define DEVM_QUERY_ACTIVE_LOCAL_DEVICE_FEATURES_REQUEST_SIZE    (sizeof(DEVM_Query_Active_Local_Device_Features_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to query the Local Device Features that are*/
   /* currently active (Response).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*          DEVM_MESSAGE_FUNCTION_QUERY_ACTIVE_LOCAL_DEVICE_FEATURES */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Query_Active_Local_Device_Features_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned long         ActiveFeatures;
} DEVM_Query_Active_Local_Device_Features_Response_t;

#define DEVM_QUERY_ACTIVE_LOCAL_DEVICE_FEATURES_RESPONSE_SIZE   (sizeof(DEVM_Query_Active_Local_Device_Features_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to perform a Role Switch (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*          DEVM_MESSAGE_FUNCTION_ROLE_SWITCH                        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Role_Switch_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DEVM_Role_t           Role;
} DEVM_Role_Switch_Request_t;

#define DEVM_ROLE_SWITCH_REQUEST_SIZE                          (sizeof(DEVM_Role_Switch_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message to perform a Role Switch (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*          DEVM_MESSAGE_FUNCTION_ROLE_SWITCH                        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Role_Switch_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} DEVM_Role_Switch_Response_t;

#define DEVM_ROLE_SWITCH_RESPONSE_SIZE                         (sizeof(DEVM_Role_Switch_Response_t))

   /* Local Device Manager Asynchronous Message Formats.                */

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device Enable/Disable state has changed (asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_ENABLE_DISABLE_CHANGE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Enable_Disable_Change_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EnableState;
   unsigned int          PreDisableTimeout;
} DEVM_Device_Enable_Disable_Change_Message_t;

#define DEVM_DEVICE_ENABLE_DISABLE_CHANGE_MESSAGE_SIZE         (sizeof(DEVM_Device_Enable_Disable_Change_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* Device Properties have changed (asynchronously).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_LOCAL_DEVICE_PROPERTIES_CHANGED */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Local_Device_Properties_Changed_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   unsigned long                  ChangedMemberMask;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;
} DEVM_Local_Device_Properties_Changed_Message_t;

#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_MESSAGE_SIZE      (sizeof(DEVM_Local_Device_Properties_Changed_Message_t))

   /* The following bit definitions are used with the ChangedMemberFlags*/
   /* member of the DEVM_Local_Device_Properties_Changed_Message_t      */
   /* structure to denote which Device Properties have changed.         */
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE   0x00000001
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME       0x00000002
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DISCOVERABLE_MODE 0x00000004
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CONNECTABLE_MODE  0x00000008
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_PAIRABLE_MODE     0x00000010
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS      0x00000020
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_APPEARANCE 0x00000040
#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_BLE_ADDRESS       0x00000080

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has started a Device Discovery operation (asynchronously). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_DISCOVERY_STARTED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Discovery_Started_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Discovery_Started_Message_t;

#define DEVM_DEVICE_DISCOVERY_STARTED_MESSAGE_SIZE             (sizeof(DEVM_Device_Discovery_Started_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has stopped a Device Discovery operation (asynchronously). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_DISCOVERY_STOPPED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Discovery_Stopped_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Discovery_Stopped_Message_t;

#define DEVM_DEVICE_DISCOVERY_STOPPED_MESSAGE_SIZE             (sizeof(DEVM_Device_Discovery_Stopped_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device has been found (asynchronously).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_FOUND             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Found_Message_t
{
   BTPM_Message_Header_t           MessageHeader;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Remote_Device_Found_Message_t;

#define DEVM_REMOTE_DEVICE_FOUND_MESSAGE_SIZE                  (sizeof(DEVM_Remote_Device_Found_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device has been deleted (asynchronously).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_DELETED           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Deleted_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} DEVM_Remote_Device_Deleted_Message_t;

#define DEVM_REMOTE_DEVICE_DELETED_MESSAGE_SIZE                (sizeof(DEVM_Remote_Device_Deleted_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's properties have changed (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_PROPERTIES_CHANGED*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Properties_Changed_Message_t
{
   BTPM_Message_Header_t           MessageHeader;
   unsigned long                   ChangedMemberMask;
   unsigned long                   ChangedRemoteDeviceFlagsMask;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Remote_Device_Properties_Changed_Message_t;

#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_MESSAGE_SIZE     (sizeof(DEVM_Remote_Device_Properties_Changed_Message_t))

   /* The following bit definitions are used with the ChangedMemberFlags*/
   /* member of the DEVM_Remote_Device_Properties_Changed_Message_t     */
   /* structure to denote which Device Properties have changed.         */
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE          0x00000001
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME              0x00000002
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_APPLICATION_DATA         0x00000004
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS             0x00000008
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_RSSI                     0x00000010
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PAIRING_STATE            0x00000020
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CONNECTION_STATE         0x00000040
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_ENCRYPTION_STATE         0x00000080
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SNIFF_STATE              0x00000100
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SERVICES_STATE           0x00000200
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_RSSI                  0x00000400
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE         0x00000800
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE      0x00001000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE      0x00002000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS 0x00004000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_APPEARANCE        0x00008000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE        0x00010000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_OBSERVED         0x00020000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASSIC_LAST_OBSERVED    0x00040000
#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_ADV_PACKET       0x00080000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's Property Query operation has completed (asynchronously). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_PROPERTIES_STATUS */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Properties_Status_Message_t
{
   BTPM_Message_Header_t           MessageHeader;
   Boolean_t                       Success;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Remote_Device_Properties_Status_Message_t;

#define DEVM_REMOTE_DEVICE_PROPERTIES_STATUS_MESSAGE_SIZE      (sizeof(DEVM_Remote_Device_Properties_Status_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's Service Discovery operation has completed                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_SERVICES_STATUS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Services_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         StatusFlags;
} DEVM_Remote_Device_Services_Status_Message_t;

#define DEVM_REMOTE_DEVICE_SERVICES_STATUS_MESSAGE_SIZE        (sizeof(DEVM_Remote_Device_Services_Status_Message_t))

   /* The following bit definitions are used with the StatusFlags member*/
   /* of the DEVM_Remote_Device_Services_Status_Message_t structure to  */
   /* denote the status of the service discovery operation.             */
#define DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_SUCCESS       0x00000001
#define DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_LOW_ENERGY    0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's pairing operation has completed (asynchronously).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_PAIRING_STATUS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Pairing_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Success;
   unsigned int          AuthenticationStatus;
} DEVM_Remote_Device_Pairing_Status_Message_t;

#define DEVM_REMOTE_DEVICE_PAIRING_STATUS_MESSAGE_SIZE         (sizeof(DEVM_Remote_Device_Pairing_Status_Message_t))

   /* The following bit definitions are used with the                   */
   /* AuthenticationStatus member of the                                */
   /* DEVM_Remote_Device_Pairing_Status_Message_t structure to denote   */
   /* the status of the service discovery operation.                    */
#define DEVM_REMOTE_DEVICE_PAIRING_STATUS_FLAGS_LOW_ENERGY     0x80000000

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's authentication operation has completed (asynchronously). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_AUTHENT_STATUS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Authentication_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   int                   Status;
} DEVM_Remote_Device_Authentication_Status_Message_t;

#define DEVM_REMOTE_DEVICE_AUTHENTICATION_STATUS_MESSAGE_SIZE  (sizeof(DEVM_Remote_Device_Authentication_Status_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's encryption operation has completed (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_ENCRYPTION_STATUS */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Encryption_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   int                   Status;
} DEVM_Remote_Device_Encryption_Status_Message_t;

#define DEVM_REMOTE_DEVICE_ENCRYPTION_STATUS_MESSAGE_SIZE      (sizeof(DEVM_Remote_Device_Encryption_Status_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's connection operation has completed (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_REMOTE_DEVICE_CONNECTION_STATUS */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Remote_Device_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   int                   Status;
} DEVM_Remote_Device_Connection_Status_Message_t;

#define DEVM_REMOTE_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE      (sizeof(DEVM_Remote_Device_Connection_Status_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has started a Device Scan operation (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_SCAN_STARTED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Scan_Started_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Scan_Started_Message_t;

#define DEVM_DEVICE_SCAN_STARTED_MESSAGE_SIZE                  (sizeof(DEVM_Device_Scan_Started_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has stopped a Device Scan operation (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_SCAN_STOPPED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Scan_Stopped_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Scan_Stopped_Message_t;

#define DEVM_DEVICE_SCAN_STOPPED_MESSAGE_SIZE                  (sizeof(DEVM_Device_Scan_Stopped_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the BD_ADDR   */
   /* that a device is referenced by has changed (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_ADDRESS_CHANGED          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Address_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   BD_ADDR_t             PreviousRemoteDeviceAddress;
} DEVM_Device_Address_Changed_Message_t;

#define DEVM_DEVICE_ADDRESS_CHANGED_MESSAGE_SIZE               (sizeof(DEVM_Device_Address_Changed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has started a Advertising operation (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_ADVERTISING_STARTED      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Advertising_Started_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Advertising_Started_Message_t;

#define DEVM_DEVICE_ADVERTISING_STARTED_MESSAGE_SIZE           (sizeof(DEVM_Device_Advertising_Started_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has stopped a Advertising operation (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_DEVICE_ADVERTISING_STOPPED      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Advertising_Stopped_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Advertising_Stopped_Message_t;

#define DEVM_DEVICE_ADVERTISING_STOPPED_MESSAGE_SIZE           (sizeof(DEVM_Device_Advertising_Stopped_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client a Advertising      */
   /* timeout event has occurred (asynchronously).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_ADVERTISING_TIMEOUT             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Advertising_Timeout_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Advertising_Timeout_Message_t;

#define DEVM_ADVERTISING_TIMEOUT_MESSAGE_SIZE                  (sizeof(DEVM_Advertising_Timeout_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has started a device observation scan operation            */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           DEVM_MESSAGE_FUNCTION_DEVICE_OBSERVATION_SCAN_STARTED   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Observation_Scan_Started_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ObservationScanFlags;
} DEVM_Device_Observation_Scan_Started_Message_t;

#define DEVM_DEVICE_OBSERVATION_SCAN_STARTED_MESSAGE_SIZE      (sizeof(DEVM_Device_Observation_Scan_Started_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that the local     */
   /* device has stopped a device observation scan operation            */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            DEVM_MESSAGE_FUNCTION_DEVICE_OBSERVATION_SCAN_STOPPED  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Observation_Scan_Stopped_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned long         ObservationScanFlags;
} DEVM_Device_Observation_Scan_Stopped_Message_t;

#define DEVM_DEVICE_OBSERVATION_SCAN_STOPPED_MESSAGE_SIZE      (sizeof(DEVM_Device_Observation_Scan_Stopped_Message_t))

   /* The following structure represents the Message definition for a   */
   /* device manager message that informs the client that an interleaved*/
   /* advertisement has been completed (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*   DEVM_MESSAGE_FUNCTION_DEVICE_INTERLEAVED_ADVERTISEMENT_COMPLETE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Interleaved_Advertisement_Complete_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DeviceManagerCallbackID;
   unsigned int          InterleavedAdvertisementID;
   int                   Status;
} DEVM_Device_Interleaved_Advertisement_Complete_Message_t;

#define DEVM_DEVICE_INTERLEAVED_ADVERTISEMENT_COMPLETE_MESSAGE_SIZE  (sizeof(DEVM_Device_Interleaved_Advertisement_Complete_Message_t))

   /* The following structure represents the Message definition for a   */
   /* device manager message that informs the client that the scheduling*/
   /* of interleaved advertisements has been suspended (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*    DEVM_MESSAGE_FUNCTION_DEVICE_INTERLEAVED_ADVERTISING_SUSPENDED */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Interleaved_Advertising_Suspended_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Interleaved_Advertising_Suspended_Message_t;

#define DEVM_DEVICE_INTERLEAVED_ADVERTISING_SUSPENDED_MESSAGE_SIZE   (sizeof(DEVM_Device_Interleaved_Advertising_Suspended_Message_t))

   /* The following structure represents the Message definition for a   */
   /* device manager message that informs the client that the scheduling*/
   /* of interleaved advertisements has been resumed (asynchronously).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*    DEVM_MESSAGE_FUNCTION_DEVICE_INTERLEAVED_ADVERTISING_RESUMED   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Device_Interleaved_Advertising_Resumed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
} DEVM_Device_Interleaved_Advertising_Resumed_Message_t;

#define DEVM_DEVICE_INTERLEAVED_ADVERTISING_RESUMED_MESSAGE_SIZE     (sizeof(DEVM_Device_Interleaved_Advertising_Resumed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Device Manager Message that informs the client that a Remote      */
   /* device's pairing action is required (asynchronously).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             DEVM_MESSAGE_FUNCTION_AUTHENTICATION_EVENT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagDEVM_Authentication_Event_Message_t
{
   BTPM_Message_Header_t             MessageHeader;
   unsigned int                      AuthenticationHandlerID;
   DEVM_Authentication_Information_t AuthenticationInformation;
} DEVM_Authentication_Event_Message_t;

#define DEVM_AUTHENTICATION_EVENT_MESSAGE_SIZE                 (sizeof(DEVM_Authentication_Event_Message_t))

#endif
