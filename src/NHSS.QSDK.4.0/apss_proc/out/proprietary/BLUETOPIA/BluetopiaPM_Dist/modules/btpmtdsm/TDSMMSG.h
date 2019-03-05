/*****< tdsmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSMMSG - Defined Interprocess Communication Messages for the 3D Sync     */
/*            (TDS) Manager for Stonestreet One Bluetopia Protocol Stack      */
/*            Platform Manager.                                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __TDSMMSGH__
#define __TDSMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTTDS.h"            /* 3D Sync Framework Prototypes/Constants.   */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "TDSMType.h"            /* BTPM 3D Sync Manager Type Definitions.    */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the 3D Sync (TDS)   */
   /* Manager.                                                          */
#define BTPM_MESSAGE_GROUP_3D_SYNC_MANAGER                              0x00001110

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the 3D Sync (TDS)    */
   /* Manager.                                                          */

   /* 3D Sync (TDS) Manager Commands.                                   */
#define TDSM_MESSAGE_FUNCTION_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS    0x00001001
#define TDSM_MESSAGE_FUNCTION_START_SYNCHRONIZATION_TRAIN               0x00001002
#define TDSM_MESSAGE_FUNCTION_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST     0x00001003
#define TDSM_MESSAGE_FUNCTION_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST    0x00001004
#define TDSM_MESSAGE_FUNCTION_GET_CURRENT_BROADCAST_INFORMATION         0x00001005
#define TDSM_MESSAGE_FUNCTION_UPDATE_BROADCAST_INFORMATION              0x00001006

#define TDSM_MESSAGE_FUNCTION_REGISTER_EVENTS                           0x00002001
#define TDSM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS                        0x00002002

   /* 3D Sync (TDS) Manager Asynchronous Events.                        */
#define TDSM_MESSAGE_FUNCTION_DISPLAY_CONNECTION_ANNOUNCEMENT           0x00010001
#define TDSM_MESSAGE_FUNCTION_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE    0x00010002
#define TDSM_MESSAGE_FUNCTION_DISPLAY_CSB_SUPERVISION_TIMEOUT           0x00010003
#define TDSM_MESSAGE_FUNCTION_DISPLAY_CHANNEL_MAP_CHANGE                0x00010004
#define TDSM_MESSAGE_FUNCTION_DISPLAY_SLAVE_PAGE_RESPONSE_TIMEOUT       0x00010005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the 3D Sync (TDS) Manager.  */

   /* 3D Sync (TDS) Manager Manager Command/Response Message Formats.   */

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to write the synchonization train parameters */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_WRITE_SYNCHRONIZATION_TRAIN     */
   /*                _PARAMETERS                                        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Write_Synchronization_Train_Parameters_Request_t
{
   BTPM_Message_Header_t                   MessageHeader;
   unsigned int                            TDSMControlCallbackID;
   TDSM_Synchronization_Train_Parameters_t SyncTrainParams;
} TDSM_Write_Synchronization_Train_Parameters_Request_t;

#define TDSM_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS_REQUEST_SIZE        (sizeof(TDSM_Write_Synchronization_Train_Parameters_Request_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to write the synchonization train parameters */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_WRITE_SYNCHRONIZATION_TRAIN     */
   /*                _PARAMETERS                                        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Write_Synchronization_Train_Parameters_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
   Word_t                IntervalResult;
} TDSM_Write_Synchronization_Train_Parameters_Response_t;

#define TDSM_WRITE_SYNCHRONIZATION_TRAIN_PARAMETERS_RESPONSE_SIZE       (sizeof(TDSM_Write_Synchronization_Train_Parameters_Response_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to start the synchronization train (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_START_SYNCHRONIZATION_TRAIN     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Start_Synchronization_Train_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          TDSMControlCallbackID;
} TDSM_Start_Synchronization_Train_Request_t;

#define TDSM_START_SYNCHRONIZATION_TRAIN_REQUEST_SIZE                   (sizeof(TDSM_Start_Synchronization_Train_Request_t))

   /* The following structure represents the message definition for     */
   /* a 3D Sync Manager message to start the synchronization train      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_START_SYNCHRONIZATION_TRAIN     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Start_Synchronization_Train_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} TDSM_Start_Synchronization_Train_Response_t;

#define TDSM_START_SYNCHRONIZATION_TRAIN_RESPONSE_SIZE                  (sizeof(TDSM_Start_Synchronization_Train_Response_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to enable the connectionless slave broadcasts*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_ENABLE_CONNECTIONLESS_SLAVE     */
   /*                _BROADCAST                                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Enable_Connectionless_Slave_Broadcast_Request_t
{
   BTPM_Message_Header_t                            MessageHeader;
   unsigned int                                     TDSMControlCallbackID;
   TDSM_Connectionless_Slave_Broadcast_Parameters_t ConnectionlessSlaveBroadcastParams;
} TDSM_Enable_Connectionless_Slave_Broadcast_Request_t;

#define TDSM_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST_REQUEST_SIZE         (sizeof(TDSM_Enable_Connectionless_Slave_Broadcast_Request_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to enable the connectionless slave broadcasts*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_ENABLE_CONNECTIONLESS_SLAVE     */
   /*                _BROADCAST                                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Enable_Connectionless_Slave_Broadcast_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
   Word_t                IntervalResult;
} TDSM_Enable_Connectionless_Slave_Broadcast_Response_t;

#define TDSM_ENABLE_CONNECTIONLESS_SLAVE_BROADCAST_RESPONSE_SIZE        (sizeof(TDSM_Enable_Connectionless_Slave_Broadcast_Response_t))

   /* The following structure represents the message definition for     */
   /* a 3D Sync Manager message to disable the connectionless slave     */
   /* broadcasts (Request).                                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISABLE_CONNECTIONLESS_SLAVE    */
   /*                _BROADCAST                                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Disable_Connectionless_Slave_Broadcast_Request_t
{
   BTPM_Message_Header_t                           MessageHeader;
   unsigned int                                    TDSMControlCallbackID;
} TDSM_Disable_Connectionless_Slave_Broadcast_Request_t;

#define TDSM_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST_REQUEST_SIZE        (sizeof(TDSM_Disable_Connectionless_Slave_Broadcast_Request_t))

   /* The following structure represents the message definition for     */
   /* a 3D Sync Manager message to disable the connectionless slave     */
   /* broadcasts (Response).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISABLE_CONNECTIONLESS_SLAVE    */
   /*                _BROADCAST                                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Disable_Connectionless_Slave_Broadcast_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} TDSM_Disable_Connectionless_Slave_Broadcast_Response_t;

#define TDSM_DISABLE_CONNECTIONLESS_SLAVE_BROADCAST_RESPONSE_SIZE       (sizeof(TDSM_Disable_Connectionless_Slave_Broadcast_Response_t))

   /* The following structure represents the message definition for a   */
   /* 3D Sync Manager message to get the current broadcast information  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_GET_CURRENT_BROADCAST           */
   /*                _INFORMATION                                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Get_Current_Broadcast_Information_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
} TDSM_Get_Current_Broadcast_Information_Request_t;

#define TDSM_GET_CURRENT_BROADCAST_INFORMATION_REQUEST_SIZE             (sizeof(TDSM_Get_Current_Broadcast_Information_Request_t))

   /* The following structure represents the message definition for a   */
   /* 3D Sync Manager message to get the current broadcast information  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_GET_CURRENT_BROADCAST           */
   /*                _INFORMATION                                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Get_Current_Broadcast_Information_Response_t
{
   BTPM_Message_Header_t                MessageHeader;
   unsigned int                         Status;
   TDSM_Current_Broadcast_Information_t CurrentBroadcastInformation;
} TDSM_Get_Current_Broadcast_Information_Response_t;

#define TDSM_GET_CURRENT_BROADCAST_INFORMATION_RESPONSE_SIZE            (sizeof(TDSM_Get_Current_Broadcast_Information_Response_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to update the current broadcast information  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_UPDATE_BROADCAST_INFORMATION    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Update_Broadcast_Information_Request_t
{
   BTPM_Message_Header_t               MessageHeader;
   unsigned int                        TDSMControlCallbackID;
   TDSM_Broadcast_Information_Update_t BroadcastInformationUpdate;
} TDSM_Update_Broadcast_Information_Request_t;

#define TDSM_UPDATE_BROADCAST_INFORMATION_REQUEST_SIZE                  (sizeof(TDSM_Update_Broadcast_Information_Request_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to update the current broadcast information  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_UPDATE_BROADCAST_INFORMATION    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Update_Broadcast_Information_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} TDSM_Update_Broadcast_Information_Response_t;

#define TDSM_UPDATE_BROADCAST_INFORMATION_RESPONSE_SIZE                 (sizeof(TDSM_Update_Broadcast_Information_Response_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to register for events.  (Request).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_REGISTER_EVENTS                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Register_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   Boolean_t             Control;
} TDSM_Register_Events_Request_t;

#define TDSM_REGISTER_EVENTS_REQUEST_SIZE                               (sizeof(TDSM_Register_Events_Request_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to register for events.  (Response).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_REGISTER_EVENTS                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Register_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned              Status;
} TDSM_Register_Events_Response_t;

#define TDSM_REGISTER_EVENTS_RESPONSE_SIZE                              (sizeof(TDSM_Register_Events_Response_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to un-register events (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Un_Register_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          TDSManagerEventCallbackID;
} TDSM_Un_Register_Events_Request_t;

#define TDSM_UN_REGISTER_EVENTS_REQUEST_SIZE                            (sizeof(TDSM_Un_Register_Events_Request_t))

   /* The following structure represents the message definition for a 3D*/
   /* Sync Manager message to un-register events (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Un_Register_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} TDSM_Un_Register_Events_Response_t;

#define TDSM_UN_REGISTER_EVENTS_RESPONSE_SIZE                           (sizeof(TDSM_Un_Register_Events_Response_t))

   /* 3D Sync Manager Asynchronous Message Formats.                     */

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that a connection */
   /* announcement has arrived for the display (asynchronously).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISPLAY_CONNECTION_ANNOUNCEMENT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Display_Connection_Announcement_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             BD_ADDR;
   unsigned int          Flags;
   unsigned int          BatteryLevel;
} TDSM_Display_Connection_Announcement_Message_t;

#define TDSM_DISPLAY_CONNECTION_ANNOUNCEMENT_MESSAGE_SIZE               (sizeof(TDSM_Display_Connection_Announcement_Message_t))

   /* The following define the valid bitmask values of the Flags field  */
   /* in the TDSM_Display_Connection_Announcement_Data_t structure.     */
#define TDSM_CONNECTION_ANNOUNCEMENT_FLAG_CONNECTION_FROM_ASSOCIATION   0x01
#define TDSM_CONNECTION_ANNOUNCEMENT_FLAG_BATTERY_LEVEL_DISPLAY_REQUEST 0x02

   /* The following defines the special value in the BatteryLevel field */
   /* of the TDSM_Display_Connection_Announcement_Data_t structure that */
   /* indicates that Battery Level Reporting is not supporting.         */
#define TDSM_CONNECTION_ANNOUNCEMENT_BATTERY_LEVEL_REPORTING_NOT_SUPPORTED 255

   /* The following structure represents the message definition for     */
   /* a Headset Manager message that informs the client that the        */
   /* synchronization train period ends (asynchronously).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISPLAY_SYNCHRONIZATION_TRAIN   */
   /*                _COMPLETE                                          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Display_Synchronization_Train_Complete_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} TDSM_Display_Synchronization_Train_Complete_Message_t;

#define TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_MESSAGE_SIZE        (sizeof(TDSM_Display_Synchronization_Train_Complete_Message_t))

#define TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_STATUS_SUCCESS      0x00000000

   /* The following structure represents the message definition for     */
   /* a Headset Manager message that informs the client that the        */
   /* connectionless slave broadcast times out (asynchronously).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISPLAY_CSB_SUPERVISION_TIMEOUT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Display_CSB_Supervision_Timeout_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
} TDSM_Display_CSB_Supervision_Timeout_Message_t;

#define TDSM_DISPLAY_CSB_SUPERVISION_TIMEOUT_MESSAGE_SIZE               (sizeof(TDSM_Display_CSB_Supervision_Timeout_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that the channel  */
   /* map changes (asynchronously).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISPLAY_CHANNEL_MAP_CHANGE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Display_Channel_Map_Change_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   AFH_Channel_Map_t     ChannelMap;
} TDSM_Display_Channel_Map_Change_Message_t;

#define TDSM_DISPLAY_CHANNEL_MAP_CHANGE_MESSAGE_SIZE                    (sizeof(TDSM_Display_Channel_Map_Change_Message_t))

   /* The following structure represents the message definition for a   */
   /* Headset Manager message that informs the client that the display  */
   /* receives a truncated page request (asynchronously).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             TDSM_MESSAGE_FUNCTION_DISPLAY_SLAVE_PAGE_RESPONSE     */
   /*                _TIMEOUT                                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagTDSM_Display_Slave_Page_Response_Timeout_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
} TDSM_Display_Slave_Page_Response_Timeout_Message_t;

#define TDSM_DISPLAY_SLAVE_PAGE_RESPONSE_TIMEOUT_MESSAGE_SIZE           (sizeof(TDSM_Display_Slave_Page_Response_Timeout_Message_t))

#endif
