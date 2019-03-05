/*****< TIPMmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMMSG - Defined Interprocess Communication Messages for the Time        */
/*            Profile (TIP) Manager for Stonestreet One Bluetopia Protocol    */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __TIPMMSGH__
#define __TIPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTTIPM.h"           /* TIP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "TIPMType.h"            /* BTPM TIP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Time Profile    */
   /* (TIP) Manager.                                                    */
#define BTPM_MESSAGE_GROUP_TIME_MANAGER                        0x00001105

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Time (TIP)       */
   /* Manager.                                                          */

   /* Time Profile (TIP) Manager Commands.                              */
#define TIPM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS           0x00001001
#define TIPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS        0x00001002
#define TIPM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS           0x00001003
#define TIPM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS        0x00001004

#define TIPM_MESSAGE_FUNCTION_SET_LOCAL_TIME_INFORMATION       0x00001103
#define TIPM_MESSAGE_FUNCTION_UPDATE_CURRENT_TIME              0x00001104
#define TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_RESPONSE          0x00001105

#define TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME                 0x00001201
#define TIPM_MESSAGE_FUNCTION_ENABLE_TIME_NOTIFICATIONS        0x00001202
#define TIPM_MESSAGE_FUNCTION_GET_LOCAL_TIME_INFORMATION       0x00001203
#define TIPM_MESSAGE_FUNCTION_GET_TIME_ACCURACY                0x00001204
#define TIPM_MESSAGE_FUNCTION_GET_NEXT_DST_CHANGE_INFORMATION  0x00001205
#define TIPM_MESSAGE_FUNCTION_GET_REFERENCE_TIME_UPDATE_STATE  0x00001206
#define TIPM_MESSAGE_FUNCTION_REQUEST_REFERENCE_TIME_UPDATE    0x00001207
#define TIPM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES          0x00001208

   /* Time Profile (TIP) Manager Asynchronous Events.                   */
#define TIPM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define TIPM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002

#define TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_REQUEST           0x00011003

#define TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME_RESPONSE        0x00012001
#define TIPM_MESSAGE_FUNCTION_CURRENT_TIME_NOTIFICATION        0x00012002
#define TIPM_MESSAGE_FUNCTION_LOCAL_TIME_INFORMATION_RESPONSE  0x00012003
#define TIPM_MESSAGE_FUNCTION_TIME_ACCURACY_RESPONSE           0x00012004
#define TIPM_MESSAGE_FUNCTION_NEXT_DST_CHANGE_RESPONSE         0x00012005
#define TIPM_MESSAGE_FUNCTION_TIME_UPDATE_STATE_RESPONSE       0x00012006

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Time Profile (TIP)      */
   /* Manager.                                                          */

   /* Time Profile (TIP) Manager Command/Response Message Formats.      */

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to register for TIP Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Register_Server_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} TIPM_Register_Server_Events_Request_t;

#define TIPM_REGISTER_SERVER_EVENTS_REQUEST_SIZE               (sizeof(TIPM_Register_Server_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to register for TIP Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Register_Server_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ServerEventHandlerID;
} TIPM_Register_Server_Events_Response_t;

#define TIPM_REGISTER_SERVER_EVENTS_RESPONSE_SIZE              (sizeof(TIPM_Register_Server_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to un-register for TIP Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Un_Register_Server_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
} TIPM_Un_Register_Server_Events_Request_t;

#define TIPM_UN_REGISTER_SERVER_EVENTS_REQUEST_SIZE            (sizeof(TIPM_Un_Register_Server_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to un-register for TIP Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Un_Register_Server_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Un_Register_Server_Events_Response_t;

#define TIPM_UN_REGISTER_SERVER_EVENTS_RESPONSE_SIZE           (sizeof(TIPM_Un_Register_Server_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to register for TIP Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Register_Client_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} TIPM_Register_Client_Events_Request_t;

#define TIPM_REGISTER_CLIENT_EVENTS_REQUEST_SIZE               (sizeof(TIPM_Register_Client_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to register for TIP Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Register_Client_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ClientEventHandlerID;
} TIPM_Register_Client_Events_Response_t;

#define TIPM_REGISTER_CLIENT_EVENTS_RESPONSE_SIZE              (sizeof(TIPM_Register_Client_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to un-register for TIP Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Un_Register_Client_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientEventHandlerID;
} TIPM_Un_Register_Client_Events_Request_t;

#define TIPM_UN_REGISTER_CLIENT_EVENTS_REQUEST_SIZE            (sizeof(TIPM_Un_Register_Client_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to un-register for TIP Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Un_Register_Client_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Un_Register_Client_Events_Response_t;

#define TIPM_UN_REGISTER_CLIENT_EVENTS_RESPONSE_SIZE           (sizeof(TIPM_Un_Register_Client_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to set the current Local Time Information     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_SET_LOCAL_TIME_INFORMATION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Set_Local_Time_Information_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ServerEventHandlerID;
   TIPM_Local_Time_Information_Data_t LocalTimeInformation;
} TIPM_Set_Local_Time_Information_Request_t;

#define TIPM_SET_LOCAL_TIME_INFORMATION_REQUEST_SIZE           (sizeof(TIPM_Set_Local_Time_Information_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to set the current Local Time Information     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_SET_LOCAL_TIME_INFORMATION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Set_Local_Time_Information_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Set_Local_Time_Information_Response_t;

#define TIPM_SET_LOCAL_TIME_INFORMATION_RESPONSE_SIZE          (sizeof(TIPM_Set_Local_Time_Information_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to force an update of the Current Time        */
   /* (Request).                                                        */

   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_UPDATE_CURRENT_TIME             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Update_Current_Time_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
   unsigned long         AdjustReasonFlags;
} TIPM_Update_Current_Time_Request_t;

#define TIPM_UPDATE_CURRENT_TIME_REQUEST_SIZE                  (sizeof(TIPM_Update_Current_Time_Request_t))

   /* The following defines the valid bits that may be set in the       */
   /* AdjustReasonFlags member of the TIPM_Update_Current_Time_Request_t*/
   /* structure.                                                        */
#define TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_MANUAL_TIME_UPDATE             0x00000001
#define TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_EXTERNAL_REFERENCE_TIME_UPDATE 0x00000002
#define TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_CHANGE_OF_TIMEZONE             0x00000004
#define TIPM_CURRENT_TIME_ADJUST_REASON_FLAGS_CHANGE_OF_DST                  0x00000008

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to force an update of the Current Time        */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_UPDATE_CURRENT_TIME             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Update_Current_Time_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Update_Current_Time_Response_t;

#define TIPM_UPDATE_CURRENT_TIME_RESPONSE_SIZE                 (sizeof(TIPM_Update_Current_Time_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to respond to a request for the current       */
   /* Reference Time Information (Request).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_RESPONSE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Reference_Time_Response_Request_t
{
   BTPM_Message_Header_t                  MessageHeader;
   unsigned int                           ServerEventHandlerID;
   TIPM_Reference_Time_Information_Data_t ReferenceTimeInformation;
} TIPM_Reference_Time_Response_Request_t;

#define TIPM_REFERENCE_TIME_RESPONSE_REQUEST_SIZE              (sizeof(TIPM_Reference_Time_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to respond to a request for the current       */
   /* Reference Time Information (Response).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_RESPONSE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Reference_Time_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Reference_Time_Response_Response_t;

#define TIPM_REFERENCE_TIME_RESPONSE_RESPONSE_SIZE             (sizeof(TIPM_Reference_Time_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the current time (Request).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Current_Time_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
} TIPM_Get_Current_Time_Request_t;

#define TIPM_GET_CURRENT_TIME_REQUEST_SIZE                     (sizeof(TIPM_Get_Current_Time_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the current time (Response).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Current_Time_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Get_Current_Time_Response_t;

#define TIPM_GET_CURRENT_TIME_RESPONSE_SIZE                    (sizeof(TIPM_Get_Current_Time_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to enable time notifications (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_ENABLE_TIME_NOTIFICATIONS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Enable_Time_Notifications_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
   Boolean_t                          Enable;
} TIPM_Enable_Time_Notifications_Request_t;

#define TIPM_ENABLE_TIME_NOTIFICATIONS_REQUEST_SIZE            (sizeof(TIPM_Enable_Time_Notifications_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to enable time notifications (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_ENABLE_TIME_NOTIFICATIONS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Enable_Time_Notifications_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Enable_Time_Notifications_Response_t;

#define TIPM_ENABLE_TIME_NOTIFICATIONS_RESPONSE_SIZE           (sizeof(TIPM_Enable_Time_Notifications_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the local time information (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_LOCAL_TIME_INFORMATION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Local_Time_Information_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
} TIPM_Get_Local_Time_Information_Request_t;

#define TIPM_GET_LOCAL_TIME_INFORMATION_REQUEST_SIZE           (sizeof(TIPM_Get_Local_Time_Information_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the local time information (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_LOCAL_TIME_INFORMATION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Local_Time_Information_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Get_Local_Time_Information_Response_t;

#define TIPM_GET_LOCAL_TIME_INFORMATION_RESPONSE_SIZE          (sizeof(TIPM_Get_Local_Time_Information_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the time accuracy (Request).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_TIME_ACCURACY               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Time_Accuracy_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
} TIPM_Get_Time_Accuracy_Request_t;

#define TIPM_GET_TIME_ACCURACY_REQUEST_SIZE                    (sizeof(TIPM_Get_Time_Accuracy_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the time accuracy (Response).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_TIME_ACCURACY               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Time_Accuracy_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Get_Time_Accuracy_Response_t;

#define TIPM_GET_TIME_ACCURACY_RESPONSE_SIZE                   (sizeof(TIPM_Get_Time_Accuracy_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the next Daylight Savings Time change  */
   /* information (Request).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_NEXT_DST_CHANGE_INFORMATION */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Next_DST_Change_Information_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
} TIPM_Get_Next_DST_Change_Information_Request_t;

#define TIPM_GET_NEXT_DST_CHANGE_INFORMATION_REQUEST_SIZE      (sizeof(TIPM_Get_Next_DST_Change_Information_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to get the next Daylight Savings Time change  */
   /* information (Response).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_NEXT_DST_CHANGE_INFORMATION */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Next_DST_Change_Information_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Get_Next_DST_Change_Information_Response_t;

#define TIPM_GET_NEXT_DST_CHANGE_INFORMATION_RESPONSE_SIZE     (sizeof(TIPM_Get_Next_DST_Change_Information_Response_t))

   /* The following structure represents the Message definition for     */
   /* a TIP Manager Message to get the reference time update state      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_REFERENCE_TIME_UPDATE_STATE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Reference_Time_Update_State_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
} TIPM_Get_Reference_Time_Update_State_Request_t;

#define TIPM_GET_REFERENCE_TIME_UPDATE_STATE_REQUEST_SIZE      (sizeof(TIPM_Get_Reference_Time_Update_State_Request_t))

   /* The following structure represents the Message definition for     */
   /* a TIP Manager Message to get the reference time update state      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_REFERENCE_TIME_UPDATE_STATE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Reference_Time_Update_State_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Get_Reference_Time_Update_State_Response_t;

#define TIPM_GET_REFERENCE_TIME_UPDATE_STATE_RESPONSE_SIZE     (sizeof(TIPM_Get_Reference_Time_Update_State_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to request a reference time update (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REQUEST_REFERENCE_TIME_UPDATE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Request_Reference_Time_Update_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   BD_ADDR_t                          RemoteDeviceAddress;
} TIPM_Request_Reference_Time_Update_Request_t;

#define TIPM_REQUEST_REFERENCE_TIME_UPDATE_REQUEST_SIZE        (sizeof(TIPM_Request_Reference_Time_Update_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to request a reference time update (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REQUEST_REFERENCE_TIME_UPDATE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Request_Reference_Time_Update_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} TIPM_Request_Reference_Time_Update_Response_t;

#define TIPM_REQUEST_REFERENCE_TIME_UPDATE_RESPONSE_SIZE       (sizeof(TIPM_Request_Reference_Time_Update_Response_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to query connected devices (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Query_Connected_Devices_Request_t
{
   BTPM_Message_Header_t              MessageHeader;
   TIPM_Connection_Type_t             ConnectionType;
} TIPM_Query_Connected_Devices_Request_t;

#define TIPM_QUERY_CONNECTED_DEVICES_REQUEST_SIZE              (sizeof(TIPM_Query_Connected_Devices_Request_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message to query connected devices (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_QUERY_CONNECTED_DEVICES         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Query_Connected_Devices_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberDevices;
   TIPM_Remote_Device_t  RemoteDevices[1];
} TIPM_Query_Connected_Devices_Response_t;

#define TIPM_QUERY_CONNECTED_DEVICES_RESPONSE_SIZE(_x)         (sizeof(TIPM_Query_Connected_Devices_Response_t) + ((_x) * (sizeof(TIPM_Remote_Device_t))))

   /* TIP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that informs the client that a remote TIP     */
   /* Connection has been made (asynchronously).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   TIPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SupportedServicesMask;
} TIPM_Connected_Message_t;

#define TIPM_CONNECTED_MESSAGE_SIZE                            (sizeof(TIPM_Connected_Message_t))

#define TIPM_SUPPORTED_SERVICES_CTS                            0x00000001
#define TIPM_SUPPORTED_SERVICES_NDCS                           0x00000002
#define TIPM_SUPPORTED_SERVICES_RTUS                           0x00000004

#define TIPM_CTS_SUPPORTED_CHARACTERISTICS_CURRENT_TIME        0x00000010
#define TIPM_CTS_SUPPORTED_CHARACTERISTICS_LOCAL_TIME_INFO     0x00000020
#define TIPM_CTS_SUPPORTED_CHARACTERISTICS_REFERENCE_TIME_INFO 0x00000040

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that informs the client that a remote TIP     */
   /* Connection has been disconnected (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   TIPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} TIPM_Disconnected_Message_t;

#define TIPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(TIPM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that informs the requests that the Client     */
   /* updates the Reference Time information (asynchronously).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_REFERENCE_TIME_REQUEST          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Reference_Time_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
} TIPM_Reference_Time_Request_Message_t;

#define TIPM_REFERENCE_TIME_REQUEST_MESSAGE_SIZE               (sizeof(TIPM_Reference_Time_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that contains the Server's response to a Get  */
   /* Current Time request                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_GET_CURRENT_TIME_RESPONSE       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Get_Current_Time_Response_Message_t
{
   BTPM_Message_Header_t    MessageHeader;
   unsigned int             ClientEventHandlerID;
   BD_ADDR_t                RemoteDeviceAddress;
   unsigned int             Status;
   TIPM_Current_Time_Data_t CurrentTimeData;
} TIPM_Get_Current_Time_Response_Message_t;

#define TIPM_GET_CURRENT_TIME_RESPONSE_MESSAGE_SIZE            (sizeof(TIPM_Get_Current_Time_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that contains a Current Time Notification from*/
   /* the TIP Server.                                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_CURRENT_TIME_NOTIFICATION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Current_Time_Notification_Message_t
{
   BTPM_Message_Header_t    MessageHeader;
   unsigned int             ClientEventHandlerID;
   BD_ADDR_t                RemoteDeviceAddress;
   TIPM_Current_Time_Data_t CurrentTimeData;
} TIPM_Current_Time_Notification_Message_t;

#define TIPM_CURRENT_TIME_NOTIFICATION_MESSAGE_SIZE            (sizeof(TIPM_Current_Time_Notification_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that contains the Server's response to a Local*/
   /* Time Information request                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_LOCAL_TIME_INFORMATION_RESPONSE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Local_Time_Information_Response_Message_t
{
   BTPM_Message_Header_t              MessageHeader;
   unsigned int                       ClientEventHandlerID;
   unsigned int                       Status;
   BD_ADDR_t                          RemoteDeviceAddress;
   TIPM_Local_Time_Information_Data_t LocalTimeInformation;
} TIPM_Local_Time_Information_Response_Message_t;

#define TIPM_LOCAL_TIME_INFORMATION_RESPONSE_MESSAGE_SIZE      (sizeof(TIPM_Local_Time_Information_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that contains the Server's response to a Time */
   /* Accuracy request                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_TIME_ACCURACY_RESPONSE          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Time_Accuracy_Response_Message_t
{
   BTPM_Message_Header_t                  MessageHeader;
   unsigned int                           ClientEventHandlerID;
   BD_ADDR_t                              RemoteDeviceAddress;
   unsigned int                           Status;
   TIPM_Reference_Time_Information_Data_t ReferenceTimeInformation;
} TIPM_Time_Accuracy_Response_Message_t;

#define TIPM_TIME_ACCURACY_RESPONSE_MESSAGE_SIZE               (sizeof(TIPM_Time_Accuracy_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that contains the Server's response to a Next */
   /* DST Change request                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_NEXT_DST_CHANGE_RESPONSE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Next_DST_Change_Response_Message_t
{
   BTPM_Message_Header_t     MessageHeader;
   unsigned int              ClientEventHandlerID;
   BD_ADDR_t                 RemoteDeviceAddress;
   unsigned int              Status;
   TIPM_Time_With_DST_Data_t TimeWithDST;
} TIPM_Next_DST_Change_Response_Message_t;

#define TIPM_NEXT_DST_CHANGE_RESPONSE_MESSAGE_SIZE             (sizeof(TIPM_Next_DST_Change_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* TIP Manager Message that contains the Server's response to a Time */
   /* Update State request                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TIPM_MESSAGE_FUNCTION_TIME_UPDATE_STATE_RESPONSE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTIPM_Time_Update_State_Response_Message_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  ClientEventHandlerID;
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  Status;
   TIPM_Time_Update_State_Data_t TimeUpdateStateData;
} TIPM_Time_Update_State_Response_Message_t;

#define TIPM_TIME_UPDATE_STATE_RESPONSE_MESSAGE_SIZE           (sizeof(TIPM_Time_Update_State_Response_Message_t))

#endif

