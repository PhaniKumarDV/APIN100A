/*****< pxpmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMMSG - Defined Interprocess Communication Messages for the Proximity   */
/*            (PXP) Manager for Stonestreet One Bluetopia Protocol Stack      */
/*            Platform Manager.                                               */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PXPMMSGH__
#define __PXPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTPXPM.h"           /* PXP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "PXPMType.h"            /* BTPM PXP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Proximity       */
   /* Profile (PXP) Manager.                                            */
#define BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER                   0x00001103

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Proximity (PXP)  */
   /* Manager.                                                          */

   /* Proximity Profile (PXP) Manager Commands.                         */
#define PXPM_MESSAGE_FUNCTION_REGISTER_MONITOR_EVENTS          0x00001001
#define PXPM_MESSAGE_FUNCTION_UN_REGISTER_MONITOR_EVENTS       0x00001002

#define PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_REFRESH_TIME       0x00001103
#define PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_THRESHOLD          0x00001104
#define PXPM_MESSAGE_FUNCTION_QUERY_CURRENT_PATH_LOSS          0x00001105
#define PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_ALERT_LEVEL        0x00001106
#define PXPM_MESSAGE_FUNCTION_SET_LINK_LOSS_ALERT_LEVEL        0x00001107

   /* Proximity Profile (PXP) Manager Asynchronous Events.              */
#define PXPM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define PXPM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002

#define PXPM_MESSAGE_FUNCTION_PATH_LOSS_ALERT                  0x00011003
#define PXPM_MESSAGE_FUNCTION_LINK_LOSS_ALERT                  0x00011004

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Proximity Profile (PXP) */
   /* Manager.                                                          */

   /* Proximity Profile (PXP) Manager Command/Response Message Formats. */

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to register for PXP Manager Monitor events    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_REGISTER_MONITOR_EVENTS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Register_Monitor_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} PXPM_Register_Monitor_Events_Request_t;

#define PXPM_REGISTER_MONITOR_EVENTS_REQUEST_SIZE               (sizeof(PXPM_Register_Monitor_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to register for PXP Manager Monitor events    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_REGISTER_MONITOR_EVENTS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Register_Monitor_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          MonitorEventHandlerID;
} PXPM_Register_Monitor_Events_Response_t;

#define PXPM_REGISTER_MONITOR_EVENTS_RESPONSE_SIZE              (sizeof(PXPM_Register_Monitor_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to un-register for PXP Manager monitor events */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_UN_REGISTER_MONITOR_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Un_Register_Monitor_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MonitorEventHandlerID;
} PXPM_Un_Register_Monitor_Events_Request_t;

#define PXPM_UN_REGISTER_MONITOR_EVENTS_REQUEST_SIZE            (sizeof(PXPM_Un_Register_Monitor_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to un-register for PXP Manager monitor events */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_UN_REGISTER_MONITOR_EVENTS      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Un_Register_Monitor_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PXPM_Un_Register_Monitor_Events_Response_t;

#define PXPM_UN_REGISTER_MONITOR_EVENTS_RESPONSE_SIZE           (sizeof(PXPM_Un_Register_Monitor_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Path Loss Refresh Time (Request).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_REFRESH_TIME      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Path_Loss_Refresh_Time_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MonitorEventHandlerID;
   unsigned int          RefreshTime;
} PXPM_Set_Path_Loss_Refresh_Time_Request_t;

#define PXPM_SET_PATH_LOSS_REFRESH_TIME_REQUEST_SIZE           (sizeof(PXPM_Set_Path_Loss_Refresh_Time_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Path Loss Refresh Time (Response). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_REFRESH_TIME      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Path_Loss_Refresh_Time_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PXPM_Set_Path_Loss_Refresh_Time_Response_t;

#define PXPM_SET_PATH_LOSS_REFRESH_TIME_RESPONSE_SIZE          (sizeof(PXPM_Set_Path_Loss_Refresh_Time_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Path Loss Threshold (Request).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_THRESHOLD         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Path_Loss_Threshold_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MonitorEventHandlerID;
   BD_ADDR_t             RemoteDevice;
   int                   PathLossThreshold;
} PXPM_Set_Path_Loss_Threshold_Request_t;

#define PXPM_SET_PATH_LOSS_THRESHOLD_REQUEST_SIZE              (sizeof(PXPM_Set_Path_Loss_Threshold_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Path Loss Threshold (Response).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_THRESHOLD         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Path_Loss_Threshold_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PXPM_Set_Path_Loss_Threshold_Response_t;

#define PXPM_SET_PATH_LOSS_THRESHOLD_RESPONSE_SIZE             (sizeof(PXPM_Set_Path_Loss_Threshold_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to query the current Path Loss (Request).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_QUERY_CURRENT_PATH_LOSS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Query_Current_Path_Loss_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MonitorEventHandlerID;
   BD_ADDR_t             RemoteDevice;
} PXPM_Query_Current_Path_Loss_Request_t;

#define PXPM_QUERY_CURRENT_PATH_LOSS_REQUEST_SIZE              (sizeof(PXPM_Query_Current_Path_Loss_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to query the current Path Loss (Response).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_QUERY_CURRENT_PATH_LOSS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Query_Current_Path_Loss_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   int                   CurrentPathLoss;
} PXPM_Query_Current_Path_Loss_Response_t;

#define PXPM_QUERY_CURRENT_PATH_LOSS_RESPONSE_SIZE             (sizeof(PXPM_Query_Current_Path_Loss_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Path Loss Alert Level (Request).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_ALERT_LEVEL       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Path_Loss_Alert_Level_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MonitorEventHandlerID;
   BD_ADDR_t             RemoteDevice;
   PXPM_Alert_Level_t    AlertLevel;
} PXPM_Set_Path_Loss_Alert_Level_Request_t;

#define PXPM_SET_PATH_LOSS_ALERT_LEVEL_REQUEST_SIZE            (sizeof(PXPM_Set_Path_Loss_Alert_Level_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Path Loss Alert Level (Response).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_ALERT_LEVEL       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Path_Loss_Alert_Level_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PXPM_Set_Path_Loss_Alert_Level_Response_t;

#define PXPM_SET_PATH_LOSS_ALERT_LEVEL_RESPONSE_SIZE           (sizeof(PXPM_Set_Path_Loss_Alert_Level_Response_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Link Loss Alert Level (Request).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_LINK_LOSS_ALERT_LEVEL       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Link_Loss_Alert_Level_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MonitorEventHandlerID;
   BD_ADDR_t             RemoteDevice;
   PXPM_Alert_Level_t    AlertLevel;
} PXPM_Set_Link_Loss_Alert_Level_Request_t;

#define PXPM_SET_LINK_LOSS_ALERT_LEVEL_REQUEST_SIZE            (sizeof(PXPM_Set_Link_Loss_Alert_Level_Request_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message to set the Link Loss Alert Level (Response).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_SET_LINK_LOSS_ALERT_LEVEL       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Set_Link_Loss_Alert_Level_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PXPM_Set_Link_Loss_Alert_Level_Response_t;

#define PXPM_SET_LINK_LOSS_ALERT_LEVEL_RESPONSE_SIZE           (sizeof(PXPM_Set_Link_Loss_Alert_Level_Response_t))

   /* PXP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message that informs the monitor that a remote PXP    */
   /* Connection has been made (asynchronously).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned long          SupportedFeatures;
} PXPM_Connected_Message_t;

#define PXPM_CONNECTED_MESSAGE_SIZE                            (sizeof(PXPM_Connected_Message_t))

   /* The following bit definitions are used with the SupportedFeatures */
   /* member of the PXPM_Connected_Message_t structure to denote the    */
   /* supported features of the PXP Connection.                         */
#define PXPM_SUPPORTED_FEATURES_FLAGS_LINK_LOSS_ALERT          0x00000001
#define PXPM_SUPPORTED_FEATURES_FLAGS_PATH_LOSS_ALERT          0x00000002

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message that informs the monitor that a remote PXP    */
   /* Connection has been disconnected (asynchronously).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} PXPM_Disconnected_Message_t;

#define PXPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(PXPM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message that informs the a Path Loss Alert has        */
   /* occurred for the specified PXP Connection (asynchronously).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_PATH_LOSS_ALERT                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Path_Loss_Alert_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   PXPM_Alert_Level_t     AlertLevel;
} PXPM_Path_Loss_Alert_Message_t;

#define PXPM_PATH_LOSS_ALERT_MESSAGE_SIZE                      (sizeof(PXPM_Path_Loss_Alert_Message_t))

   /* The following structure represents the Message definition for a   */
   /* PXP Manager Message that informs the a Link Loss Alert has        */
   /* occurred for the specified PXP Connection (asynchronously).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PXPM_MESSAGE_FUNCTION_LINK_LOSS_ALERT                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPXPM_Link_Loss_Alert_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   PXPM_Alert_Level_t     AlertLevel;
} PXPM_Link_Loss_Alert_Message_t;

#define PXPM_LINK_LOSS_ALERT_MESSAGE_SIZE                      (sizeof(PXPM_Link_Loss_Alert_Message_t))

#endif

