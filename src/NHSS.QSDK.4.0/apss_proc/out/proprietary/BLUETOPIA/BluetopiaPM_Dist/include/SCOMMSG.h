/*****< scommsg.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SCOMMSG - Defined Interprocess Communication Messages for the SCO         */
/*            Manager for Stonestreet One Bluetopia Protocol Stack Platform   */
/*            Manager.                                                        */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/07/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SCOMMSGH__
#define __SCOMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the SCO Manager.    */
#define BTPM_MESSAGE_GROUP_SCO_CONNECTION_MANAGER              0x00000120

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the SCO Manager.     */

   /* SCO Manager Commands.                                             */
#define SCOM_MESSAGE_FUNCTION_REGISTER_SERVER_CONNECTION       0x00001001
#define SCOM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_CONNECTION    0x00001002
#define SCOM_MESSAGE_FUNCTION_ENABLE_SERVER_CONNECTION         0x00001003
#define SCOM_MESSAGE_FUNCTION_OPEN_REMOTE_CONNECTION           0x00001004
#define SCOM_MESSAGE_FUNCTION_CLOSE_CONNECTION                 0x00001005

   /* SCO Manager Asynchronous Events.                                  */
#define SCOM_MESSAGE_FUNCTION_SERVER_CONNECTION_OPENED         0x00010001
#define SCOM_MESSAGE_FUNCTION_CONNECTION_CLOSED                0x00010002
#define SCOM_MESSAGE_FUNCTION_OPEN_REMOTE_CONNECTION_RESULT    0x00010003

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the SCO Manager.            */

   /* SCO Manager Command/Response Message Formats.                     */

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Register a Local SCO Connection (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_REGISTER_SERVER_CONNECTION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Register_Server_Connection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   Boolean_t             EnableConnection;
   BD_ADDR_t             RemoteDevice;
} SCOM_Register_Server_Connection_Request_t;

#define SCOM_REGISTER_SERVER_CONNECTION_REQUEST_SIZE           (sizeof(SCOM_Register_Server_Connection_Request_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Register a Local Server (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_REGISTER_SERVER_CONNECTION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Register_Server_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ConnectionID;
} SCOM_Register_Server_Connection_Response_t;

#define SCOM_REGISTER_SERVER_CONNECTION_RESPONSE_SIZE          (sizeof(SCOM_Register_Server_Connection_Response_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Un-Register a Local SCO Server (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_CONNECTION   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Un_Register_Server_Connection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
} SCOM_Un_Register_Server_Connection_Request_t;

#define SCOM_UN_REGISTER_SERVER_CONNECTION_REQUEST_SIZE        (sizeof(SCOM_Un_Register_Server_Connection_Request_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Un-Register a Local Server (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_CONNECTION   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Un_Register_Server_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SCOM_Un_Register_Server_Connection_Response_t;

#define SCOM_UN_REGISTER_SERVER_CONNECTION_RESPONSE_SIZE       (sizeof(SCOM_Un_Register_Server_Connection_Response_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Enable a Local SCO Server (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_ENABLE_SERVER_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Enable_Server_Connection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   Boolean_t             EnableConnection;
   BD_ADDR_t             RemoteDevice;
} SCOM_Enable_Server_Connection_Request_t;

#define SCOM_ENABLE_SERVER_CONNECTION_REQUEST_SIZE             (sizeof(SCOM_Enable_Server_Connection_Request_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Enable a Local SCO Server (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_ENABLE_SERVER_CONNECTION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Enable_Server_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SCOM_Enable_Server_Connection_Response_t;

#define SCOM_ENABLE_SERVER_CONNECTION_RESPONSE_SIZE            (sizeof(SCOM_Enable_Server_Connection_Response_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Open a Remote SCO Connection (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_OPEN_REMOTE_CONNECTION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Open_Remote_Connection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDevice;
} SCOM_Open_Remote_Connection_Request_t;

#define SCOM_OPEN_REMOTE_CONNECTION_REQUEST_SIZE               (sizeof(SCOM_Open_Remote_Connection_Request_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Open a Remote SCO Connection (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_OPEN_REMOTE_CONNECTION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Open_Remote_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ConnectionID;
} SCOM_Open_Remote_Connection_Response_t;

#define SCOM_OPEN_REMOTE_CONNECTION_RESPONSE_SIZE              (sizeof(SCOM_Open_Remote_Connection_Response_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Close an active SCO Connection (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_CLOSE_CONNECTION                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Close_Connection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
} SCOM_Close_Connection_Request_t;

#define SCOM_CLOSE_CONNECTION_REQUEST_SIZE                     (sizeof(SCOM_Close_Connection_Request_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message to Close an active SCO Connection (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_CLOSE_CONNECTION                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Close_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SCOM_Close_Connection_Response_t;

#define SCOM_CLOSE_CONNECTION_RESPONSE_SIZE                    (sizeof(SCOM_Close_Connection_Response_t))

   /* SCO Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message that informs the client that a SCO Server     */
   /* connection is now active (i.e. connected - asynchronously).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_SERVER_CONNECTION_OPENED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Server_Connection_Opened_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   BD_ADDR_t             RemoteDeviceAddress;
} SCOM_Server_Connection_Opened_Message_t;

#define SCOM_SERVER_CONNECTION_OPENED_MESSAGE_SIZE             (sizeof(SCOM_Server_Connection_Opened_Message_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message that informs the client that a SCO connection */
   /* is no longer active (i.e. disconnected - asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_CONNECTION_CLOSED               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Connection_Closed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
} SCOM_Connection_Closed_Message_t;

#define SCOM_CONNECTION_CLOSED_MESSAGE_SIZE                    (sizeof(SCOM_Connection_Closed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* SCO Manager Message that informs the client of the Result of a    */
   /* remote SCO connection that was initiated from the local client    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SCOM_MESSAGE_FUNCTION_OPEN_REMOTE_CONNECTION_RESULT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSCOM_Open_Remote_Connection_Result_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   int                   Status;
} SCOM_Open_Remote_Connection_Result_Message_t;

#define SCOM_OPEN_REMOTE_CONNECTION_RESULT_MESSAGE_SIZE        (sizeof(SCOM_Open_Remote_Connection_Result_Message_t))

#endif
