/*****< basmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BASMMSG - Defined Interprocess Communication Messages for the Battery     */
/*            Service (BAS) Manager for Stonestreet One Bluetopia Protocol    */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BASMMSGH__
#define __BASMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTBASM.h"           /* BAS Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "BASMType.h"            /* BTPM BAS Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */

   /* Platform Manager Message Group that specifies the Battery Service */
   /* (BAS) Manager.                                                    */
#define BTPM_MESSAGE_GROUP_BATTERY_SERVICE_MANAGER                0x00001107

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Alert            */
   /* Notification (BAS) Manager.                                       */

   /* Battery Service (BAS) Manager Commands.                           */
#define BASM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS              0x00001001
#define BASM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS           0x00001002

#define BASM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS                0x00001103
#define BASM_MESSAGE_FUNCTION_DISABLE_NOTIFICATIONS               0x00001104
#define BASM_MESSAGE_FUNCTION_GET_BATTERY_LEVEL                   0x00001105
#define BASM_MESSAGE_FUNCTION_GET_BATTERY_IDENTIFICATION          0x00001106
#define BASM_MESSAGE_FUNCTION_CANCEL_TRANSACTION                  0x00001107

   /* Battery Service (BAS) Manager Asynchronous Events.                */
#define BASM_MESSAGE_FUNCTION_CONNECTED                           0x00010001
#define BASM_MESSAGE_FUNCTION_DISCONNECTED                        0x00010002

#define BASM_MESSAGE_FUNCTION_BATTERY_LEVEL                       0x00011003
#define BASM_MESSAGE_FUNCTION_BATTERY_LEVEL_NOTIFICATION          0x00011004
#define BASM_MESSAGE_FUNCTION_BATTERY_IDENTIFICATION              0x00011005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Battery Service (BAS)   */
   /* Manager.                                                          */

   /* Battery Service (BAS) Manager Manager Request/Response Message    */
   /* Formats.                                                          */

   /* The following structure represents the Message definition for a   */
   /* common BAS Manager Message Request.                               */
   /* * NOTE * This is the message format for the following Message     */
   /*          Function ID's:                                           */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS            */
   /*             BASM_MESSAGE_FUNCTION_DISABLE_NOTIFICATIONS           */
   /*             BASM_MESSAGE_FUNCTION_GET_BATTERY_LEVEL               */
   /*             BASM_MESSAGE_FUNCTION_GET_BATTERY_IDENTIFICATION      */
typedef struct _tagBASM_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} BASM_Request_Message_t;

#define BASM_REQUEST_MESSAGE_SIZE                 (sizeof(BASM_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* common BAS Manager Message Response.                              */
   /* * NOTE * This is the message format for the following Message     */
   /*          Function ID's:                                           */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS       */
   /*             BASM_MESSAGE_FUNCTION_ENABLE_NOTIFICATIONS            */
   /*             BASM_MESSAGE_FUNCTION_DISABLE_NOTIFICATIONS           */
   /*             BASM_MESSAGE_FUNCTION_GET_BATTERY_LEVEL               */
   /*             BASM_MESSAGE_FUNCTION_GET_BATTERY_IDENTIFICATION      */
   /*             BASM_MESSAGE_FUNCTION_CANCEL_TRANSACTION              */
typedef struct _tagBASM_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} BASM_Response_Message_t;

#define BASM_RESPONSE_MESSAGE_SIZE                 (sizeof(BASM_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message to register for BAS Client events (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_REGISTER_CLIENT_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Register_Client_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} BASM_Register_Client_Events_Request_t;

#define BASM_REGISTER_CLIENT_EVENTS_REQUEST_SIZE            (sizeof(BASM_Register_Client_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message to un-register for BAS Client events          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_UN_REGISTER_CLIENT_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Un_Register_Client_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} BASM_Un_Register_Client_Events_Request_t;

#define BASM_UN_REGISTER_CLIENT_EVENTS_REQUEST_SIZE         (sizeof(BASM_Un_Register_Client_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message to send the Cancel Transaction message        */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_CANCEL_TRANSACTION              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Cancel_Transaction_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          TransactionID;
} BASM_Cancel_Transaction_Request_t;

#define BASM_CANCEL_TRANSACTION_REQUEST_SIZE        (sizeof(BASM_Cancel_Transaction_Request_t))

   /* BAS Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message that informs the client that a BAS device has */
   /* connected (asynchronously).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   BASM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
   unsigned int           NumberOfInstances;
} BASM_Connected_Message_t;

#define BASM_CONNECTED_MESSAGE_SIZE                         (sizeof(BASM_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message that informs the client that a BAS device has */
   /* disconnected (asynchronously).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   BASM_Connection_Type_t ConnectionType;
} BASM_Disconnected_Message_t;

#define BASM_DISCONNECTED_MESSAGE_SIZE                      (sizeof(BASM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message that informs the client that the Battery Level*/
   /* has been sent by a server (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_BATTERY_LEVEL                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Battery_Level_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          TransactionID;
   int                   Status;
   Byte_t                BatteryLevel;
} BASM_Battery_Level_Message_t;

#define BASM_BATTERY_LEVEL_MESSAGE_SIZE                     (sizeof(BASM_Battery_Level_Message_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message that informs the client that the Battery Level*/
   /* has been sent by a server (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_BATTERY_LEVEL_NOTIFICATION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Battery_Level_Notification_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Byte_t                BatteryLevel;
} BASM_Battery_Level_Notification_Message_t;

#define BASM_BATTERY_LEVEL_NOTIFICATION_MESSAGE_SIZE                     (sizeof(BASM_Battery_Level_Notification_Message_t))

   /* The following structure represents the Message definition for a   */
   /* BAS Manager Message that informs the client that a Battery        */
   /* Identification Response has been sent by a server                 */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BASM_MESSAGE_FUNCTION_BATTERY_IDENTIFICATION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBASM_Battery_Identification_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   unsigned int          TransactionID;
   int                   Status;
   Byte_t                Namespace;
   Word_t                Description;
} BASM_Battery_Identification_Message_t;

#define BASM_BATTERY_IDENTIFICATION_MESSAGE_SIZE            (sizeof(BASM_Battery_Identification_Message_t))

#endif
