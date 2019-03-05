/*****< anpmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMMSG - Defined Interprocess Communication Messages for the Alert       */
/*            Notification Profile (ANP) Manager for Stonestreet One          */
/*            Bluetopia Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANPMMSGH__
#define __ANPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTANPM.h"           /* ANP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "ANPMType.h"            /* BTPM ANP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Alert           */
   /* Notification Profile (ANP) Manager.                               */
#define BTPM_MESSAGE_GROUP_ALERT_NOTIFICATION_MANAGER          0x00001101

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Alert            */
   /* Notification (ANP) Manager.                                       */

   /* Alert Notification Profile (ANP) Manager Commands.                */
#define ANPM_MESSAGE_FUNCTION_REGISTER_ANP_EVENTS                 0x00001001
#define ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_EVENTS              0x00001002

#define ANPM_MESSAGE_FUNCTION_SET_NEW_ALERT                       0x00001103
#define ANPM_MESSAGE_FUNCTION_SET_UN_READ_ALERT                   0x00001104

   /* ANP Client Commands.                                              */
#define ANPM_MESSAGE_FUNCTION_REGISTER_ANP_CLIENT_EVENTS          0x00002001
#define ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_CLIENT_EVENTS       0x00002002

#define ANPM_MESSAGE_FUNCTION_GET_SUPPORTED_CATEGORIES            0x00002101
#define ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_NOTIFICATIONS        0x00002102
#define ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_CATEGORY             0x00002103
#define ANPM_MESSAGE_FUNCTION_REQUEST_NOTIFICATION                0x00002104

   /* Alert Notification Profile (ANP) Manager Asynchronous Events.     */
#define ANPM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define ANPM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002

#define ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_ENABLED       0x00011003
#define ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_DISABLED      0x00011004
#define ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_ENABLED   0x00011005
#define ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_DISABLED  0x00011006

   /* ANP Client Asynchronous Events. */
#define ANPM_MESSAGE_FUNCTION_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT  0x00020001
#define ANPM_MESSAGE_FUNCTION_SUPPORTED_UNREAD_CATEGORIES_RESULT     0x00020002
#define ANPM_MESSAGE_FUNCTION_NEW_ALERT_NOTIFICATION                 0x00020003
#define ANPM_MESSAGE_FUNCTION_UNREAD_STATUS_NOTIFICATION             0x00020004
#define ANPM_MESSAGE_FUNCTION_COMMAND_RESULT                         0x00020005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Alert Notification      */
   /* Profile (ANP) Manager.                                            */

   /* Alert Notification Profile (ANP) Manager Command/Response Message */
   /* Formats.                                                          */

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to register for ANP Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_REGISTER_ANP_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Register_ANP_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} ANPM_Register_ANP_Events_Request_t;

#define ANPM_REGISTER_ANP_EVENTS_REQUEST_SIZE                  (sizeof(ANPM_Register_ANP_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to register for ANP Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_REGISTER_ANP_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Register_ANP_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Register_ANP_Events_Response_t;

#define ANPM_REGISTER_ANP_EVENTS_RESPONSE_SIZE                 (sizeof(ANPM_Register_ANP_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to un-register for ANP Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Un_Register_ANP_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} ANPM_Un_Register_ANP_Events_Request_t;

#define ANPM_UN_REGISTER_ANP_EVENTS_REQUEST_SIZE               (sizeof(ANPM_Un_Register_ANP_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to un-register for ANP Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Un_Register_ANP_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Un_Register_ANP_Events_Response_t;

#define ANPM_UN_REGISTER_ANP_EVENTS_RESPONSE_SIZE              (sizeof(ANPM_Un_Register_ANP_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to send ANP New Alerts (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_SET_NEW_ALERT                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Set_New_Alert_Request_t
{
   BTPM_Message_Header_t          MessageHeader;
   ANPM_Category_Identification_t CategoryID;
   unsigned int                   NewAlertCount;
   unsigned int                   LastAlertTextLength;
   char                           LastAlertText[1];
} ANPM_Set_New_Alert_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Set New Alert Request Message*/
   /* given the number of actual last alert text bytes.  This function  */
   /* accepts as it's input the total number individual report data     */
   /* bytes are present starting from the LastAlertText member of the   */
   /* ANPM_Set_New_Alert_Request_t structure and returns the total      */
   /* number of bytes required to hold the entire message.              */
#define ANPM_SET_NEW_ALERT_REQUEST_SIZE(_x)                    (STRUCTURE_OFFSET(ANPM_Set_New_Alert_Request_t, LastAlertText) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to send ANP New Alerts (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_SET_NEW_ALERT                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Set_New_Alert_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Set_New_Alert_Response_t;

#define ANPM_SET_NEW_ALERT_RESPONSE_SIZE                       (sizeof(ANPM_Set_New_Alert_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to send ANP Un-Read Alerts (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_SET_UN_READ_ALERT               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Set_Un_Read_Alert_Request_t
{
   BTPM_Message_Header_t          MessageHeader;
   ANPM_Category_Identification_t CategoryID;
   unsigned int                   UnReadAlertCount;
} ANPM_Set_Un_Read_Alert_Request_t;

#define ANPM_SET_UN_READ_ALERT_REQUEST_SIZE                    (sizeof(ANPM_Set_Un_Read_Alert_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to send ANP Un-Read Alerts (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_SET_UN_READ_ALERT               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Set_Un_Read_Alert_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Set_Un_Read_Alert_Response_t;

#define ANPM_SET_UN_READ_ALERT_RESPONSE_SIZE                   (sizeof(ANPM_Set_Un_Read_Alert_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to register for ANP Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_REGISTER_ANP_CLIENT_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Register_ANP_Client_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} ANPM_Register_ANP_Client_Events_Request_t;

#define ANPM_REGISTER_ANP_CLIENT_EVENTS_REQUEST_SIZE           (sizeof(ANPM_Register_ANP_Client_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to register for ANP Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_REGISTER_ANP_CLIENT_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Register_ANP_Client_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Register_ANP_Client_Events_Response_t;

#define ANPM_REGISTER_ANP_CLIENT_EVENTS_RESPONSE_SIZE          (sizeof(ANPM_Register_ANP_Client_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to un-register for ANP Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_CLIENT_EVENTS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Un_Register_ANP_Client_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientCallbackID;
} ANPM_Un_Register_ANP_Client_Events_Request_t;

#define ANPM_UN_REGISTER_ANP_CLIENT_EVENTS_REQUEST_SIZE        (sizeof(ANPM_Un_Register_ANP_Client_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to un-register for ANP Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UN_REGISTER_ANP_CLIENT_EVENTS   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Un_Register_ANP_Client_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Un_Register_ANP_Client_Events_Response_t;

#define ANPM_UN_REGISTER_ANP_CLIENT_EVENTS_RESPONSE_SIZE       (sizeof(ANPM_Un_Register_ANP_Client_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to get supported categories for a notification*/
   /* type (Request).                                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_GET_SUPPORTED_CATEGORIES        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Get_Supported_Categories_Request_t
{
   BTPM_Message_Header_t    MessageHeader;
   unsigned int             ClientCallbackID;
   BD_ADDR_t                RemoteDeviceAddress;
   ANPM_Notification_Type_t NotificationType;
} ANPM_Get_Supported_Categories_Request_t;

#define ANPM_GET_SUPPORTED_CATEGORIES_REQUEST_SIZE             (sizeof(ANPM_Get_Supported_Categories_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to get supported categories for a notification*/
   /* type (Response).                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_GET_SUPPORTED_CATEGORIES        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Get_Supported_Categories_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Get_Supported_Categories_Response_t;

#define ANPM_GET_SUPPORTED_CATEGORIES_RESPONSE_SIZE            (sizeof(ANPM_Get_Supported_Categories_Response_t))

   /* The following structure represents the Message definition for     */
   /* a ANP Manager Message to enable or disable notifications for a    */
   /* notification type (Request).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_NOTIFICATIONS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Enable_Disable_Notifications_Request_t
{
   BTPM_Message_Header_t    MessageHeader;
   unsigned int             ClientCallbackID;
   BD_ADDR_t                RemoteDeviceAddress;
   ANPM_Notification_Type_t NotificationType;
   Boolean_t                Enable;
} ANPM_Enable_Disable_Notifications_Request_t;

#define ANPM_ENABLE_DISABLE_NOTIFICATIONS_REQUEST_SIZE         (sizeof(ANPM_Enable_Disable_Notifications_Request_t))

   /* The following structure represents the Message definition for     */
   /* a ANP Manager Message to enable or disable notifications for a    */
   /* notification type (Response).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_NOTIFICATIONS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Enable_Disable_Notifications_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Enable_Disable_Notifications_Response_t;

#define ANPM_ENABLE_DISABLE_NOTIFICATIONS_RESPONSE_SIZE        (sizeof(ANPM_Enable_Disable_Notifications_Response_t))

   /* The following structure represents the Message definition for     */
   /* a ANP Manager Message to enable or disable a category for a       */
   /* notification type (Request).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_CATEGORY         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Enable_Disable_Category_Request_t
{
   BTPM_Message_Header_t          MessageHeader;
   unsigned int                   ClientCallbackID;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Notification_Type_t       NotificationType;
   Boolean_t                      Enable;
   ANPM_Category_Identification_t CategoryID;
} ANPM_Enable_Disable_Category_Request_t;

#define ANPM_ENABLE_DISABLE_CATEGORY_REQUEST_SIZE              (sizeof(ANPM_Enable_Disable_Category_Request_t))

   /* The following structure represents the Message definition for     */
   /* a ANP Manager Message to enable or disable a category for a       */
   /* notification type (Response).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_ENABLE_DISABLE_CATEGORY         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Enable_Disable_Category_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Enable_Disable_Category_Response_t;

#define ANPM_ENABLE_DISABLE_CATEGORY_RESPONSE_SIZE             (sizeof(ANPM_Enable_Disable_Category_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to request notification for a given category  */
   /* for a notification type (Request).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_REQUEST_NOTIFICATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Request_Notification_Request_t
{
   BTPM_Message_Header_t          MessageHeader;
   unsigned int                   ClientCallbackID;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Notification_Type_t       NotificationType;
   ANPM_Category_Identification_t CategoryID;
} ANPM_Request_Notification_Request_t;

#define ANPM_REQUEST_NOTIFICATION_REQUEST_SIZE                 (sizeof(ANPM_Request_Notification_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message to request notification for a given category  */
   /* for a notification type (Response).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_REQUEST_NOTIFICATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Request_Notification_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANPM_Request_Notification_Response_t;

#define ANPM_REQUEST_NOTIFICATION_RESPONSE_SIZE                (sizeof(ANPM_Request_Notification_Response_t))

   /* ANP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Client Connection has been made (asynchronously).                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   ANPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} ANPM_Connected_Message_t;

#define ANPM_CONNECTED_MESSAGE_SIZE                            (sizeof(ANPM_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Client Connection has been disconnected (asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   ANPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} ANPM_Disconnected_Message_t;

#define ANPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(ANPM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Client has enabled New Alert Categories (asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_ENABLED      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_New_Alert_Category_Enabled_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryEnabled;
   Word_t                         EnabledCategories;
} ANPM_New_Alert_Category_Enabled_Message_t;

#define ANPM_NEW_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE           (sizeof(ANPM_New_Alert_Category_Enabled_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Client has disabled New Alert Categories (asynchronously).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_NEW_ALERT_CATEGORY_DISABLED     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_New_Alert_Category_Disabled_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryDisabled;
   Word_t                         EnabledCategories;
} ANPM_New_Alert_Category_Disabled_Message_t;

#define ANPM_NEW_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE          (sizeof(ANPM_New_Alert_Category_Disabled_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Client has enabled Un-Read Alert Categories (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_ENABLED  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Un_Read_Alert_Category_Enabled_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryEnabled;
   Word_t                         EnabledCategories;
} ANPM_Un_Read_Alert_Category_Enabled_Message_t;

#define ANPM_UN_READ_ALERT_CATEGORY_ENABLED_MESSAGE_SIZE       (sizeof(ANPM_Un_Read_Alert_Category_Enabled_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Client has disabled Un-Read Alert Categories (asynchronously).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UN_READ_ALERT_CATEGORY_DISABLED */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Un_Read_Alert_Category_Disabled_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryDisabled;
   Word_t                         EnabledCategories;
} ANPM_Un_Read_Alert_Category_Disabled_Message_t;

#define ANPM_UN_READ_ALERT_CATEGORY_DISABLED_MESSAGE_SIZE      (sizeof(ANPM_Un_Read_Alert_Category_Disabled_Message_t))

   /* ANP Client Asynchronous Messages.                                 */

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Server has response to a Get Supported New Alert Categories       */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_SUPPORTED_NEW_ALERT_CATEGORIES  */
   /*                _RESULT                                            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Supported_New_Alert_Categories_Result_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   unsigned int          Status;
   unsigned int          AttProtocolErrorCode;
   unsigned long         SupportedCategories;
} ANPM_Supported_New_Alert_Categories_Result_Message_t;

#define ANPM_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT_MESSAGE_SIZE   (sizeof(ANPM_Supported_New_Alert_Categories_Result_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Server has responded to Get Supported Unread Status Categories    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_SUPPORTED_UNREAD_CATEGORIES     */
   /*                _RESULT                                            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Supported_Unread_Categories_Result_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   unsigned int          Status;
   unsigned int          AttProtocolErrorCode;
   unsigned long         SupportedCategories;
} ANPM_Supported_Unread_Categories_Result_Message_t;

#define ANPM_SUPPORTED_UNREAD_CATEGORIES_RESULT_MESSAGE_SIZE   (sizeof(ANPM_Supported_Unread_Categories_Result_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Server has sent a New Alert notification (asynchronously).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_NEW_ALERT_NOTIFICATION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_New_Alert_Notification_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryID;
   unsigned int                   NumberNewAlerts;
   unsigned int                   LastAlertTextLength;
   char                           LastAlertText[1];
} ANPM_New_Alert_Notification_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Set New Alert Request Message*/
   /* given the number of actual last alert text bytes.  This function  */
   /* accepts as it's input the total number individual report data     */
   /* bytes are present starting from the LastAlertText member of the   */
   /* ANPM_Set_New_Alert_Request_t structure and returns the total      */
   /* number of bytes required to hold the entire message.              */
#define ANPM_NEW_ALERT_NOTIFICATION_MESSAGE_SIZE(_x)           (STRUCTURE_OFFSET(ANPM_New_Alert_Notification_Message_t, LastAlertText) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Server has sent an Unread Status notification (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_UNREAD_STATUS_NOTIFICATION      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Unread_Status_Notification_Message_t
{
   BTPM_Message_Header_t          MessageHeader;
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryID;
   unsigned int                   NumberUnreadAlerts;
} ANPM_Unread_Status_Notification_Message_t;

#define ANPM_UNREAD_STATUS_NOTIFICATION_MESSAGE_SIZE           (sizeof(ANPM_Unread_Status_Notification_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANP Manager Message that informs the client that a remote ANP     */
   /* Server has responded to control point command write request       */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANPM_MESSAGE_FUNCTION_COMMAND_RESULT                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANPM_Command_Result_Message_t
{
   BTPM_Message_Header_t             MessageHeader;
   BD_ADDR_t                         RemoteDeviceAddress;
   unsigned int                      TransactionID;
   unsigned int                      Status;
   unsigned int                      AttProtocolErrorCode;
} ANPM_Command_Result_Message_t;

#define ANPM_COMMAND_RESULT_MESSAGE_SIZE                       (sizeof(ANPM_Command_Result_Message_t))

#endif

