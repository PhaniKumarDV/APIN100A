/*****< pasmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMMSG - Defined Interprocess Communication Messages for the Phone       */
/*            Alert Status (PAS) Manager for Stonestreet One Bluetopia        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __PASMMSGH__
#define __PASMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "PASMType.h"            /* BTPM PAS Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Phone Alert     */
   /* Status (PAS) Manager.                                             */
#define BTPM_MESSAGE_GROUP_PHONE_ALERT_STATUS_MANAGER          0x00001106

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Phone Alert      */
   /* Status (PAS) Manager.                                             */

   /* Phone Alert Status (PAS) Manager Commands.                        */
#define PASM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS           0x00001001
#define PASM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS        0x00001002

#define PASM_MESSAGE_FUNCTION_SET_ALERT_STATUS                 0x00001101
#define PASM_MESSAGE_FUNCTION_QUERY_ALERT_STATUS               0x00001102
#define PASM_MESSAGE_FUNCTION_SET_RINGER_SETTING               0x00001103
#define PASM_MESSAGE_FUNCTION_QUERY_RINGER_SETTING             0x00001104

   /* Phone Alert Status (PAS) Manager Asynchronous Events.             */
#define PASM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define PASM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002

#define PASM_MESSAGE_FUNCTION_RINGER_CONTROL_POINT_COMMAND     0x00011001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Phone Alert Status (PAS)*/
   /* Manager.                                                          */

   /* Phone Alert Status (PAS) Manager Command/Response Message Formats.*/

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to register for PAS Manager Server events     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Register_Server_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} PASM_Register_Server_Events_Request_t;

#define PASM_REGISTER_SERVER_EVENTS_REQUEST_SIZE               (sizeof(PASM_Register_Server_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to register for PAS Manager Server events     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_REGISTER_SERVER_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Register_Server_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ServerEventHandlerID;
} PASM_Register_Server_Events_Response_t;

#define PASM_REGISTER_SERVER_EVENTS_RESPONSE_SIZE              (sizeof(PASM_Register_Server_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to un-register for PAS Manager Server events  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Un_Register_Server_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
} PASM_Un_Register_Server_Events_Request_t;

#define PASM_UN_REGISTER_SERVER_EVENTS_REQUEST_SIZE            (sizeof(PASM_Un_Register_Server_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to un-register for PAS Manager server events  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Un_Register_Server_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PASM_Un_Register_Server_Events_Response_t;

#define PASM_UN_REGISTER_SERVER_EVENTS_RESPONSE_SIZE           (sizeof(PASM_Un_Register_Server_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to set the current alert status (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_SET_ALERT_STATUS                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Set_Alert_Status_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
   PASM_Alert_Status_t   AlertStatus;
} PASM_Set_Alert_Status_Request_t;

#define PASM_SET_ALERT_STATUS_REQUEST_SIZE                     (sizeof(PASM_Set_Alert_Status_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to set the current alert status (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_SET_ALERT_STATUS                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Set_Alert_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PASM_Set_Alert_Status_Response_t;

#define PASM_SET_ALERT_STATUS_RESPONSE_SIZE                    (sizeof(PASM_Set_Alert_Status_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to query the current alert status (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_QUERY_ALERT_STATUS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Query_Alert_Status_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
} PASM_Query_Alert_Status_Request_t;

#define PASM_QUERY_ALERT_STATUS_REQUEST_SIZE                   (sizeof(PASM_Query_Alert_Status_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to query the current alert status (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_QUERY_ALERT_STATUS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Query_Alert_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   PASM_Alert_Status_t   AlertStatus;
} PASM_Query_Alert_Status_Response_t;

#define PASM_QUERY_ALERT_STATUS_RESPONSE_SIZE                  (sizeof(PASM_Query_Alert_Status_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to set the current ringer setting (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_SET_RINGER_SETTING              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Set_Ringer_Setting_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
   PASM_Ringer_Setting_t RingerSetting;
} PASM_Set_Ringer_Setting_Request_t;

#define PASM_SET_RINGER_SETTING_REQUEST_SIZE                   (sizeof(PASM_Set_Ringer_Setting_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to set the current ringer setting (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_SET_RINGER_SETTING              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Set_Ringer_Setting_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PASM_Set_Ringer_Setting_Response_t;

#define PASM_SET_RINGER_SETTING_RESPONSE_SIZE                  (sizeof(PASM_Set_Ringer_Setting_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to query the current ringer setting (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_QUERY_RINGER_SETTING            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Query_Ringer_Setting_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerEventHandlerID;
} PASM_Query_Ringer_Setting_Request_t;

#define PASM_QUERY_RINGER_SETTING_REQUEST_SIZE                 (sizeof(PASM_Query_Ringer_Setting_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message to query the current ringer setting           */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_QUERY_RINGER_SETTING            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Query_Ringer_Setting_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   PASM_Ringer_Setting_t RingerSetting;
} PASM_Query_Ringer_Setting_Response_t;

#define PASM_QUERY_RINGER_SETTING_RESPONSE_SIZE                (sizeof(PASM_Query_Ringer_Setting_Response_t))

   /* Phone Alert Status (PAS) Manager Asynchronous Message Formats.    */

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message that informs the server that a remote PAS     */
   /* Client connection has been made (asynchronously).                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   PASM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} PASM_Connected_Message_t;

#define PASM_CONNECTED_MESSAGE_SIZE                            (sizeof(PASM_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message that informs the Server that a remote PAS     */
   /* Client connection has been disconnected (asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   PASM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} PASM_Disconnected_Message_t;

#define PASM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(PASM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* PAS Manager Message that informs that the remote connected device */
   /* has sent a ringer control point command on the specified PAS      */
   /* connection (asynchronously).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PASM_MESSAGE_FUNCTION_RINGER_CONTROL_POINT_COMMAND    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPASM_Ringer_Control_Point_Command_Message_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  ServerEventHandlerID;
   BD_ADDR_t                     RemoteDeviceAddress;
   PASM_Ringer_Control_Command_t RingerControlCommand;
} PASM_Ringer_Control_Point_Command_Message_t;

#define PASM_RINGER_CONTROL_POINT_COMMAND_MESSAGE_SIZE         (sizeof(PASM_Ringer_Control_Point_Command_Message_t))

#endif

