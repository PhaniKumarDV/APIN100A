/*****< antmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTMMSG - Defined Interprocess Communication Messages for the ANT+        */
/*            Manager for Stonestreet One Bluetopia Protocol Stack Platform   */
/*            Manager.                                                        */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/30/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANTMMSGH__
#define __ANTMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTANTM.h"           /* ANT Framework Prototypes/Constants.       */

#include "SS1BTANT.h"            /* Bluetopia ANT+ Prototypes/Constants.      */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the ANT+ Manager.   */
#define BTPM_MESSAGE_GROUP_ANT_PLUS_MANAGER                             0x00001009

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the ANT+ Manager.    */

   /* ANT+ Manager Commands.                                            */
#define ANTM_MESSAGE_FUNCTION_REGISTER_ANT_EVENTS                       0x00001001
#define ANTM_MESSAGE_FUNCTION_UN_REGISTER_ANT_EVENTS                    0x00001002

#define ANTM_MESSAGE_FUNCTION_ASSIGN_CHANNEL                            0x00001101
#define ANTM_MESSAGE_FUNCTION_UN_ASSIGN_CHANNEL                         0x00001102
#define ANTM_MESSAGE_FUNCTION_SET_CHANNEL_ID                            0x00001103
#define ANTM_MESSAGE_FUNCTION_SET_CHANNEL_PERIOD                        0x00001104
#define ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_TIMEOUT                0x00001105
#define ANTM_MESSAGE_FUNCTION_SET_CHANNEL_RF_FREQUENCY                  0x00001106
#define ANTM_MESSAGE_FUNCTION_SET_NETWORK_KEY                           0x00001107
#define ANTM_MESSAGE_FUNCTION_SET_TRANSMIT_POWER                        0x00001108
#define ANTM_MESSAGE_FUNCTION_ADD_CHANNEL_ID                            0x00001109
#define ANTM_MESSAGE_FUNCTION_CONFIGURE_INCL_EXCL_LIST                  0x0000110A
#define ANTM_MESSAGE_FUNCTION_SET_CHANNEL_TRANSMIT_POWER                0x0000110B
#define ANTM_MESSAGE_FUNCTION_SET_LOW_PRIORITY_CHANNEL_SEARCH_TIMEOUT   0x0000110C
#define ANTM_MESSAGE_FUNCTION_SET_SERIAL_NUMBER_CHANNEL_ID              0x0000110D
#define ANTM_MESSAGE_FUNCTION_ENABLE_EXTENDED_MESSAGES                  0x0000110E
#define ANTM_MESSAGE_FUNCTION_ENABLE_LED                                0x0000110F
#define ANTM_MESSAGE_FUNCTION_ENABLE_CRYSTAL                            0x00001110
#define ANTM_MESSAGE_FUNCTION_EXTENDED_MESSAGES_CONFIGURATION           0x00001111
#define ANTM_MESSAGE_FUNCTION_CONFIGURE_FREQUENCY_AGILITY               0x00001112
#define ANTM_MESSAGE_FUNCTION_SET_PROXIMITY_SEARCH                      0x00001113
#define ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_PRIORITY               0x00001114
#define ANTM_MESSAGE_FUNCTION_SET_USB_DESCRIPTOR_STRING                 0x00001115

#define ANTM_MESSAGE_FUNCTION_RESET_SYSTEM                              0x00001201
#define ANTM_MESSAGE_FUNCTION_OPEN_CHANNEL                              0x00001202
#define ANTM_MESSAGE_FUNCTION_CLOSE_CHANNEL                             0x00001203
#define ANTM_MESSAGE_FUNCTION_REQUEST_CHANNEL_MESSAGE                   0x00001204
#define ANTM_MESSAGE_FUNCTION_OPEN_RX_SCAN_MODE                         0x00001205
#define ANTM_MESSAGE_FUNCTION_SLEEP_MESSAGE                             0x00001206

#define ANTM_MESSAGE_FUNCTION_SEND_BROADCAST_DATA                       0x00001301
#define ANTM_MESSAGE_FUNCTION_SEND_ACKNOWLEDGED_DATA                    0x00001302
#define ANTM_MESSAGE_FUNCTION_SEND_BURST_TRANSFER_DATA                  0x00001303

#define ANTM_MESSAGE_FUNCTION_INITIALIZE_CW_TEST_MODE                   0x00001401
#define ANTM_MESSAGE_FUNCTION_SET_CW_TEST_MODE                          0x00001402

#define ANTM_MESSAGE_FUNCTION_SEND_RAW_PACKET                           0x00001501

   /* ANT+ Manager Asynchronous Events.                                 */
#define ANTM_MESSAGE_FUNCTION_STARTUP_MESSAGE                           0x00010001
#define ANTM_MESSAGE_FUNCTION_CHANNEL_RESPONSE                          0x00010002
#define ANTM_MESSAGE_FUNCTION_CHANNEL_STATUS                            0x00010003
#define ANTM_MESSAGE_FUNCTION_CHANNEL_ID                                0x00010004
#define ANTM_MESSAGE_FUNCTION_ANT_VERSION                               0x00010005
#define ANTM_MESSAGE_FUNCTION_CAPABILITIES                              0x00010006
#define ANTM_MESSAGE_FUNCTION_BROADCAST_DATA_PACKET                     0x00010007
#define ANTM_MESSAGE_FUNCTION_ACKNOWLEDGED_DATA_PACKET                  0x00010008
#define ANTM_MESSAGE_FUNCTION_BURST_DATA_PACKET                         0x00010009
#define ANTM_MESSAGE_FUNCTION_EXTENDED_BROADCAST_DATA_PACKET            0x0001000A
#define ANTM_MESSAGE_FUNCTION_EXTENDED_ACKNOWLEDGED_DATA_PACKET         0x0001000B
#define ANTM_MESSAGE_FUNCTION_EXTENDED_BURST_DATA_PACKET                0x0001000C
#define ANTM_MESSAGE_FUNCTION_RAW_DATA_PACKET                           0x0001000D

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the ANT+ Manager.           */

   /* ANT+ Manager Command/Response Message Formats.                    */

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to register for ANT Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_REGISTER_ANT_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Register_ANT_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} ANTM_Register_ANT_Events_Request_t;

#define ANTM_REGISTER_ANT_EVENTS_REQUEST_SIZE                  (sizeof(ANTM_Register_ANT_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to register for ANT Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_REGISTER_ANT_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Register_ANT_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
//xxx Added Event Handler ID
   unsigned int          EventHandlerID;
} ANTM_Register_ANT_Events_Response_t;

#define ANTM_REGISTER_ANT_EVENTS_RESPONSE_SIZE                 (sizeof(ANTM_Register_ANT_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to un-register for ANT Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_UN_REGISTER_ANT_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Un_Register_ANT_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
//xxx Added Event Handler ID
   unsigned int          EventHandlerID;
} ANTM_Un_Register_ANT_Events_Request_t;

#define ANTM_UN_REGISTER_ANT_EVENTS_REQUEST_SIZE               (sizeof(ANTM_Un_Register_ANT_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to un-register for ANT Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_UN_REGISTER_ANT_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Un_Register_ANT_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Un_Register_ANT_Events_Response_t;

#define ANTM_UN_REGISTER_ANT_EVENTS_RESPONSE_SIZE              (sizeof(ANTM_Un_Register_ANT_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to assign a channel (Request).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ASSIGN_CHANNEL                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Assign_Channel_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           ChannelType;
   unsigned int           NetworkNumber;
   unsigned int           ExtendedAssignment;
} ANTM_Assign_Channel_Request_t;

#define ANTM_ASSIGN_CHANNEL_REQUEST_SIZE                       (sizeof(ANTM_Assign_Channel_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to assign a channel (Response).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ASSIGN_CHANNEL                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Assign_Channel_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Assign_Channel_Response_t;

#define ANTM_ASSIGN_CHANNEL_RESPONSE_SIZE                      (sizeof(ANTM_Assign_Channel_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to un-assign a channel (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_UN_ASSIGN_CHANNEL               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Un_Assign_Channel_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
} ANTM_Un_Assign_Channel_Request_t;

#define ANTM_UN_ASSIGN_CHANNEL_REQUEST_SIZE                    (sizeof(ANTM_Un_Assign_Channel_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to un-assign a channel (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_UN_ASSIGN_CHANNEL               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Un_Assign_Channel_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Un_Assign_Channel_Response_t;

#define ANTM_UN_ASSIGN_CHANNEL_RESPONSE_SIZE                   (sizeof(ANTM_Un_Assign_Channel_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel ID (Request).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_ID                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_ID_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           DeviceNumber;
   unsigned int           DeviceType;
   unsigned int           TransmissionType;
} ANTM_Set_Channel_ID_Request_t;

#define ANTM_SET_CHANNEL_ID_REQUEST_SIZE                       (sizeof(ANTM_Set_Channel_ID_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel ID (Response)                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_ID                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_ID_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Channel_ID_Response_t;

#define ANTM_SET_CHANNEL_ID_RESPONSE_SIZE                      (sizeof(ANTM_Set_Channel_ID_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel period (Request).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_PERIOD              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Period_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           MessagingPeriod;
} ANTM_Set_Channel_Period_Request_t;

#define ANTM_SET_CHANNEL_PERIOD_REQUEST_SIZE                   (sizeof(ANTM_Set_Channel_Period_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel period (Response).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_PERIOD              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Period_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Channel_Period_Response_t;

#define ANTM_SET_CHANNEL_PERIOD_RESPONSE_SIZE                  (sizeof(ANTM_Set_Channel_Period_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel search timeout (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_TIMEOUT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Search_Timeout_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           SearchTimeout;
} ANTM_Set_Channel_Search_Timeout_Request_t;

#define ANTM_SET_CHANNEL_SEARCH_TIMEOUT_REQUEST_SIZE           (sizeof(ANTM_Set_Channel_Search_Timeout_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel search timeout (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_TIMEOUT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Search_Timeout_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Channel_Search_Timeout_Response_t;

#define ANTM_SET_CHANNEL_SEARCH_TIMEOUT_RESPONSE_SIZE          (sizeof(ANTM_Set_Channel_Search_Timeout_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel RF frequency (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_RF_FREQUENCY        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_RF_Frequency_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           RFFrequency;
} ANTM_Set_Channel_RF_Frequency_Request_t;

#define ANTM_SET_CHANNEL_RF_FREQUENCY_REQUEST_SIZE             (sizeof(ANTM_Set_Channel_RF_Frequency_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a channel RF frequency (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_RF_FREQUENCY        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_RF_Frequency_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Channel_RF_Frequency_Response_t;

#define ANTM_SET_CHANNEL_RF_FREQUENCY_RESPONSE_SIZE            (sizeof(ANTM_Set_Channel_RF_Frequency_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a network key (Request).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_NETWORK_KEY                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Network_Key_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           NetworkNumber;
   ANT_Network_Key_t      NetworkKey;
} ANTM_Set_Network_Key_Request_t;

#define ANTM_SET_NETWORK_KEY_REQUEST_SIZE                      (sizeof(ANTM_Set_Network_Key_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set a network key (Response).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_NETWORK_KEY                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Network_Key_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Network_Key_Response_t;

#define ANTM_SET_NETWORK_KEY_RESPONSE_SIZE                     (sizeof(ANTM_Set_Network_Key_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the transmit power level (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_TRANSMIT_POWER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Transmit_Power_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           TransmitPower;
} ANTM_Set_Transmit_Power_Request_t;

#define ANTM_SET_TRANSMIT_POWER_REQUEST_SIZE                   (sizeof(ANTM_Set_Transmit_Power_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the transmit power level (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_TRANSMIT_POWER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Transmit_Power_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Transmit_Power_Response_t;

#define ANTM_SET_TRANSMIT_POWER_RESPONSE_SIZE                  (sizeof(ANTM_Set_Transmit_Power_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for adding a Channel ID to the local device's */
   /* inclusion / exclusion list (Request).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ADD_CHANNEL_ID                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Add_Channel_ID_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          DeviceNumber;
   unsigned int          DeviceType;
   unsigned int          TransmissionType;
   unsigned int          ListIndex;
} ANTM_Add_Channel_ID_Request_t;

#define ANTM_ADD_CHANNEL_ID_REQUEST_SIZE                       (sizeof(ANTM_Add_Channel_ID_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for adding a Channel ID to the local device's */
   /* inclusion / exclusion list (Response).                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ADD_CHANNEL_ID                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Add_Channel_ID_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Add_Channel_ID_Response_t;

#define ANTM_ADD_CHANNEL_ID_RESPONSE_SIZE                      (sizeof(ANTM_Add_Channel_ID_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the local device's inclusion  */
   /* / exclusion list (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CONFIGURE_INCL_EXCL_LIST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Configure_Incl_Excl_List_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          ListSize;
   unsigned int          Exclude;
} ANTM_Configure_Incl_Excl_List_Request_t;

#define ANTM_CONFIGURE_INCL_EXCL_LIST_REQUEST_SIZE             (sizeof(ANTM_Configure_Incl_Excl_List_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the local device's inclusion  */
   /* / exclusion list (Response).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CONFIGURE_INCL_EXCL_LIST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Configure_Incl_Excl_List_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Configure_Incl_Excl_List_Response_t;

#define ANTM_CONFIGURE_INCL_EXCL_LIST_RESPONSE_SIZE            (sizeof(ANTM_Configure_Incl_Excl_List_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the transmit power level for a channel */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_TRANSMIT_POWER      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Transmit_Power_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           TransmitPower;
} ANTM_Set_Channel_Transmit_Power_Request_t;

#define ANTM_SET_CHANNEL_TRANSMIT_POWER_REQUEST_SIZE           (sizeof(ANTM_Set_Channel_Transmit_Power_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the transmit power level for a channel */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_TRANSMIT_POWER      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Transmit_Power_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Channel_Transmit_Power_Response_t;

#define ANTM_SET_CHANNEL_TRANSMIT_POWER_RESPONSE_SIZE          (sizeof(ANTM_Set_Channel_Transmit_Power_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the search timeout for a low priority  */
   /* channel scan (Request).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*     ANTM_MESSAGE_FUNCTION_SET_LOW_PRIORITY_CHANNEL_SEARCH_TIMEOUT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           SearchTimeout;
} ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t;

#define ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_REQUEST_SIZE  (sizeof(ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the search timeout for a low priority  */
   /* channel scan (Response).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*     ANTM_MESSAGE_FUNCTION_SET_LOW_PRIORITY_CHANNEL_SEARCH_TIMEOUT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Response_t;

#define ANTM_SET_LOW_PRIORITY_CHANNEL_SCAN_SEARCH_TIMEOUT_RESPONSE_SIZE (sizeof(ANTM_Set_Low_Priority_Channel_Scan_Search_Timeout_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the Channel ID using part of the serial*/
   /* number for the device's number for the channel (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_SERIAL_NUMBER_CHANNEL_ID    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Serial_Number_Channel_ID_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           DeviceType;
   unsigned int           TransmissionType;
} ANTM_Set_Serial_Number_Channel_ID_Request_t;

#define ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_REQUEST_SIZE         (sizeof(ANTM_Set_Serial_Number_Channel_ID_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the Channel ID using part of the serial*/
   /* number for the device's number for the channel (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_SERIAL_NUMBER_CHANNEL_ID    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Serial_Number_Channel_ID_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Serial_Number_Channel_ID_Response_t;

#define ANTM_SET_SERIAL_NUMBER_CHANNEL_ID_RESPONSE_SIZE        (sizeof(ANTM_Set_Serial_Number_Channel_ID_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for enabling or disabling extended Rx messages*/
   /* for an ANT Channel (Request).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ENABLE_EXTENDED_MESSAGES        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Enable_Extended_Messages_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   Boolean_t              Enable;
} ANTM_Enable_Extended_Messages_Request_t;

#define ANTM_ENABLE_EXTENDED_MESSAGES_REQUEST_SIZE             (sizeof(ANTM_Enable_Extended_Messages_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for enabling extended or disabling Rx messages*/
   /* for an ANT Channel (Response).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ENABLE_EXTENDED_MESSAGES        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Enable_Extended_Messages_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Enable_Extended_Messages_Response_t;

#define ANTM_ENABLE_EXTENDED_MESSAGES_RESPONSE_SIZE            (sizeof(ANTM_Enable_Extended_Messages_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for enabling or disabling an LED on the local */
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ENABLE_LED                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Enable_LED_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   Boolean_t              Enable;
} ANTM_Enable_LED_Request_t;

#define ANTM_ENABLE_LED_REQUEST_SIZE                           (sizeof(ANTM_Enable_LED_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for enabling or disabling an LED on the local */
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ENABLE_LED                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Enable_LED_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Enable_LED_Response_t;

#define ANTM_ENABLE_LED_RESPONSE_SIZE                          (sizeof(ANTM_Enable_LED_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for enabling a Crystal on the local device    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ENABLE_CRYSTAL                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Enable_Crystal_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
} ANTM_Enable_Crystal_Request_t;

#define ANTM_ENABLE_CRYSTAL_REQUEST_SIZE                       (sizeof(ANTM_Enable_Crystal_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for enabling a Crystal on the local device    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ENABLE_CRYSTAL                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Enable_Crystal_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Enable_Crystal_Response_t;

#define ANTM_ENABLE_CRYSTAL_RESPONSE_SIZE                      (sizeof(ANTM_Enable_Crystal_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the enabled extended Rx       */
   /* messages for the local device (Request).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            ANTM_MESSAGE_FUNCTION_EXTENDED_MESSAGES_CONFIGURATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Extended_Messages_Configuration_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           RxMessagesMask;
} ANTM_Extended_Messages_Configuration_Request_t;

#define ANTM_EXTENDED_MESSAGES_CONFIGURATION_REQUEST_SIZE      (sizeof(ANTM_Extended_Messages_Configuration_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the enabled extended Rx       */
   /* messages for the local device (Response).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            ANTM_MESSAGE_FUNCTION_EXTENDED_MESSAGES_CONFIGURATION  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Extended_Messages_Configuration_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Extended_Messages_Configuration_Response_t;

#define ANTM_EXTENDED_MESSAGES_CONFIGURATION_RESPONSE_SIZE     (sizeof(ANTM_Extended_Messages_Configuration_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the three operating           */
   /* frequencies for an ANT channel (Request).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            ANTM_MESSAGE_FUNCTION_CONFIGURE_FREQUENCY_AGILITY      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Configure_Frequency_Agility_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           FrequencyAgility1;
   unsigned int           FrequencyAgility2;
   unsigned int           FrequencyAgility3;
} ANTM_Configure_Frequency_Agility_Request_t;

#define ANTM_CONFIGURE_FREQUENCY_AGILITY_REQUEST_SIZE          (sizeof(ANTM_Configure_Frequency_Agility_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the three operating           */
   /* frequencies for an ANT channel (Response).                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            ANTM_MESSAGE_FUNCTION_CONFIGURE_FREQUENCY_AGILITY      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Configure_Frequency_Agility_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Configure_Frequency_Agility_Response_t;

#define ANTM_CONFIGURE_FREQUENCY_AGILITY_RESPONSE_SIZE         (sizeof(ANTM_Configure_Frequency_Agility_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the proximity search requirements for  */
   /* the local device (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_PROXIMITY_SEARCH            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Proximity_Search_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           SearchThreshold;
} ANTM_Set_Proximity_Search_Request_t;

#define ANTM_SET_PROXIMITY_SEARCH_REQUEST_SIZE                 (sizeof(ANTM_Set_Proximity_Search_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the proximity search requirements for  */
   /* the local device (Response).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_PROXIMITY_SEARCH            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Proximity_Search_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Proximity_Search_Response_t;

#define ANTM_SET_PROXIMITY_SEARCH_RESPONSE_SIZE                (sizeof(ANTM_Set_Proximity_Search_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the channel search priority for the    */
   /* local device (Request).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_PRIORITY     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Search_Priority_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
   unsigned int           ChannelNumber;
   unsigned int           SearchPriority;
} ANTM_Set_Channel_Search_Priority_Request_t;

#define ANTM_SET_CHANNEL_SEARCH_PRIORITY_REQUEST_SIZE          (sizeof(ANTM_Set_Channel_Search_Priority_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to set the channel search priority for the    */
   /* local device (Response).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CHANNEL_SEARCH_PRIORITY     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_Channel_Search_Priority_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_Channel_Search_Priority_Response_t;

#define ANTM_SET_CHANNEL_SEARCH_PRIORITY_RESPONSE_SIZE         (sizeof(ANTM_Set_Channel_Search_Priority_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to configure the USB descriptor string for the*/
   /* local device (Request).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_USB_DESCRIPTOR_STRING       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_USB_Descriptor_String_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          StringNumber;
   unsigned int          DescriptorStringLength;
   char                  DescriptorString[1];
} ANTM_Set_USB_Descriptor_String_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Set USB Descriptor String    */
   /* Message given the number of actual string bytes.  This function   */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the DescriptorString member of the          */
   /* ANTM_Set_USB_Descriptor_String_Request_t structure and returns the*/
   /* total number of bytes required to hold the entire message.        */
#define ANTM_SET_USB_DESCRIPTOR_STRING_REQUEST_SIZE(_x)        (STRUCTURE_OFFSET(ANTM_Set_USB_Descriptor_String_Request_t, DescriptorString) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message to configure the USB descriptor string for the*/
   /* local device (Response).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_USB_DESCRIPTOR_STRING       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_USB_Descriptor_String_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_USB_Descriptor_String_Response_t;

#define ANTM_SET_USB_DESCRIPTOR_STRING_RESPONSE_SIZE           (sizeof(ANTM_Set_USB_Descriptor_String_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for resetting the ANT system on the local     */
   /* device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_RESET_SYSTEM                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Reset_System_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           EventHandlerID;
} ANTM_Reset_System_Request_t;

#define ANTM_RESET_SYSTEM_REQUEST_SIZE                         (sizeof(ANTM_Reset_System_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for resetting the ANT system on the local     */
   /* device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_RESET_SYSTEM                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Reset_System_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Reset_System_Response_t;

#define ANTM_RESET_SYSTEM_RESPONSE_SIZE                        (sizeof(ANTM_Reset_System_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for opening an ANT+ channel (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_OPEN_CHANNEL                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Open_Channel_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
} ANTM_Open_Channel_Request_t;

#define ANTM_OPEN_CHANNEL_REQUEST_SIZE                         (sizeof(ANTM_Open_Channel_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for opening an ANT+ channel (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_OPEN_CHANNEL                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Open_Channel_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Open_Channel_Response_t;

#define ANTM_OPEN_CHANNEL_RESPONSE_SIZE                        (sizeof(ANTM_Open_Channel_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for closing an ANT+ channel (Request).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CLOSE_CHANNEL                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Close_Channel_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
} ANTM_Close_Channel_Request_t;

#define ANTM_CLOSE_CHANNEL_REQUEST_SIZE                        (sizeof(ANTM_Close_Channel_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for closing an ANT+ channel (Response).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CLOSE_CHANNEL                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Close_Channel_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Close_Channel_Response_t;

#define ANTM_CLOSE_CHANNEL_RESPONSE_SIZE                       (sizeof(ANTM_Close_Channel_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for requesting a specified information message*/
   /* for the specified channel (Request).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_REQUEST_CHANNEL_MESSAGE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Request_Channel_Message_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          MessageID;
} ANTM_Request_Channel_Message_Request_t;

#define ANTM_REQUEST_CHANNEL_MESSAGE_REQUEST_SIZE              (sizeof(ANTM_Request_Channel_Message_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for requesting a specified information message*/
   /* for the specified channel (Response).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_REQUEST_CHANNEL_MESSAGE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Request_Channel_Message_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Request_Channel_Message_Response_t;

#define ANTM_REQUEST_CHANNEL_MESSAGE_RESPONSE_SIZE             (sizeof(ANTM_Request_Channel_Message_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for opening an ANT+ channel in continous scan */
   /* mode (Request).                                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_OPEN_RX_SCAN_MODE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Open_Rx_Scan_Mode_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
} ANTM_Open_Rx_Scan_Mode_Request_t;

#define ANTM_OPEN_RX_SCAN_MODE_REQUEST_SIZE                    (sizeof(ANTM_Open_Rx_Scan_Mode_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for opening an ANT+ channel in continous scan */
   /* mode (Response).                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_OPEN_RX_SCAN_MODE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Open_Rx_Scan_Mode_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Open_Rx_Scan_Mode_Response_t;

#define ANTM_OPEN_RX_SCAN_MODE_RESPONSE_SIZE                   (sizeof(ANTM_Open_Rx_Scan_Mode_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for putting the local ANT system in low power */
   /* mode (Request).                                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SLEEP_MESSAGE                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Sleep_Message_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
} ANTM_Sleep_Message_Request_t;

#define ANTM_SLEEP_MESSAGE_REQUEST_SIZE                        (sizeof(ANTM_Sleep_Message_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for putting the local ANT system in low power */
   /* mode (Response).                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SLEEP_MESSAGE                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Sleep_Message_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Sleep_Message_Response_t;

#define ANTM_SLEEP_MESSAGE_RESPONSE_SIZE                       (sizeof(ANTM_Sleep_Message_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for sending broadcast data on the specified   */
   /* channel (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_BROADCAST_DATA             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Broadcast_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          BroadcastDataLength;
   char                  BroadcastData[1];
} ANTM_Send_Broadcast_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Send Broadcast Data Message  */
   /* given the number of actual broadcast data bytes.  This function   */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the BroadcastData member of the             */
   /* ANTM_Send_Broadcast_Data_Request_t structure and returns the total*/
   /* number of bytes required to hold the entire message.              */
#define ANTM_SEND_BROADCAST_DATA_REQUEST_SIZE(_x)              (STRUCTURE_OFFSET(ANTM_Send_Broadcast_Data_Request_t, BroadcastData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for sending broadcast data on the specified   */
   /* channel (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_BROADCAST_DATA             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Broadcast_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Send_Broadcast_Data_Response_t;

#define ANTM_SEND_BROADCAST_DATA_RESPONSE_SIZE                 (sizeof(ANTM_Send_Broadcast_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for sending Acknowledged data on the specified*/
   /* channel (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_ACKNOWLEDGED_DATA          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Acknowledged_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          AcknowledgedDataLength;
   char                  AcknowledgedData[1];
} ANTM_Send_Acknowledged_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Send Acknowledged Data       */
   /* Message given the number of actual Acknowledged data bytes.  This */
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the AcknowledgedData member of the*/
   /* ANTM_Send_Acknowledged_Data_Request_t structure and returns the   */
   /* total number of bytes required to hold the entire message.        */
#define ANTM_SEND_ACKNOWLEDGED_DATA_REQUEST_SIZE(_x)           (STRUCTURE_OFFSET(ANTM_Send_Acknowledged_Data_Request_t, AcknowledgedData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for sending Acknowledged data on the specified*/
   /* channel (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_ACKNOWLEDGED_DATA          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Acknowledged_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Send_Acknowledged_Data_Response_t;

#define ANTM_SEND_ACKNOWLEDGED_DATA_RESPONSE_SIZE              (sizeof(ANTM_Send_Acknowledged_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for sending Burst Transfer data on the        */
   /* specified channel (Request).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_BURST_TRANSFER_DATA        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Burst_Transfer_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          SequenceChannelNumber;
   unsigned int          BurstTransferDataLength;
   char                  BurstTransferData[1];
} ANTM_Send_Burst_Transfer_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Send Burst_Transfer Data     */
   /* Message given the number of actual Burst Transfer data bytes.     */
   /* This function accepts as it's input the total number individual   */
   /* data bytes are present starting from the BurstTransferData member */
   /* of the ANTM_Send_Burst_Transfer_Data_Request_t structure and      */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define ANTM_SEND_BURST_TRANSFER_DATA_REQUEST_SIZE(_x)         (STRUCTURE_OFFSET(ANTM_Send_Burst_Transfer_Data_Request_t, BurstTransferData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for sending Burst Transfer data on the        */
   /* specified channel (Response).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_BURST_TRANSFER_DATA        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Burst_Transfer_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Send_Burst_Transfer_Data_Response_t;

#define ANTM_SEND_BURST_TRANSFER_DATA_RESPONSE_SIZE            (sizeof(ANTM_Send_Burst_Transfer_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for putting the local ANT system in Continuous*/
   /* Wave test mode (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_INITIALIZE_CW_TEST_MODE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Initialize_CW_Test_Mode_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
} ANTM_Initialize_CW_Test_Mode_Request_t;

#define ANTM_INITIALIZE_CW_TEST_MODE_REQUEST_SIZE              (sizeof(ANTM_Initialize_CW_Test_Mode_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for putting the local ANT system in Continuous*/
   /* Wave test mode (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_INITIALIZE_CW_TEST_MODE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Initialize_CW_Test_Mode_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Initialize_CW_Test_Mode_Response_t;

#define ANTM_INITIALIZE_CW_TEST_MODE_RESPONSE_SIZE             (sizeof(ANTM_Initialize_CW_Test_Mode_Response_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the parameters for a          */
   /* Continuous Wave test in the local ANT+ system (Request).          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CW_TEST_MODE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_CW_Test_Mode_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          TxPower;
   unsigned int          RFFrequency;
} ANTM_Set_CW_Test_Mode_Request_t;

#define ANTM_SET_CW_TEST_MODE_REQUEST_SIZE                     (sizeof(ANTM_Set_CW_Test_Mode_Request_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for configuring the parameters for a          */
   /* Continuous Wave test in the local ANT+ system (Response).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SET_CW_TEST_MODE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Set_CW_Test_Mode_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Set_CW_Test_Mode_Response_t;

#define ANTM_SET_CW_TEST_MODE_RESPONSE_SIZE                    (sizeof(ANTM_Set_CW_Test_Mode_Response_t))

   /* The following structure represents the Message definition for     */
   /* a ANT Manager Message for sending a preformatted ANT packet       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_RAW_PACKET                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Raw_Packet_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          PacketLength;
   Byte_t                PacketData[1];
} ANTM_Send_Raw_Packet_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes     */
   /* that will be required to hold a an entire Send Raw Packet         */
   /* Message given the size of the actual packet (in bytes).  This     */
   /* function accepts as it's input the total number individual        */
   /* data bytes present starting from the PacketData member of the     */
   /* ANTM_Send_Raw_Packet_Request_t structure and returns the total    */
   /* number of bytes required to hold the entire message.              */
#define ANTM_SEND_RAW_PACKET_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(ANTM_Send_Raw_Packet_Request_t, PacketData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for     */
   /* a ANT Manager Message for sending a preformatted ANT packet       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_SEND_RAW_PACKET                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Send_Raw_Packet_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} ANTM_Send_Raw_Packet_Response_t;

#define ANTM_SEND_RAW_PACKET_RESPONSE_SIZE                     (sizeof(ANTM_Send_Raw_Packet_Response_t))

   /* ANT+ Manager Asynchronous Events.                                 */

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message that is sent by the ANT system on startup     */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_STARTUP_MESSAGE                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Startup_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          StartupMessage;
} ANTM_Startup_Message_t;

#define ANTM_STARTUP_MESSAGE_SIZE                              (sizeof(ANTM_Startup_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Channel Response event            */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CHANNEL_RESPONSE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Channel_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          MessageID;
   unsigned int          MessageCode;
} ANTM_Channel_Response_Message_t;

#define ANTM_CHANNEL_RESPONSE_MESSAGE_SIZE                     (sizeof(ANTM_Channel_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Channel Status event              */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CHANNEL_STATUS                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Channel_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          ChannelStatus;
} ANTM_Channel_Status_Message_t;

#define ANTM_CHANNEL_STATUS_MESSAGE_SIZE                       (sizeof(ANTM_Channel_Status_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Channel ID event (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CHANNEL_ID                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Channel_ID_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          DeviceNumber;
   unsigned int          DeviceTypeID;
   unsigned int          TransmissionType;
} ANTM_Channel_ID_Message_t;

#define ANTM_CHANNEL_ID_MESSAGE_SIZE                           (sizeof(ANTM_Channel_ID_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Version event (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ANT_VERSION                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_ANT_Version_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          VersionDataLength;
   Byte_t                VersionData[1];
} ANTM_ANT_Version_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Version Message given the    */
   /* number of actual Version bytes.  This function accepts as it's    */
   /* input the total number individual data bytes are present starting */
   /* from the VersionData member of the ANTM_ANT_Version_Message_t     */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define ANTM_VERSION_MESSAGE_SIZE(_x)                          (STRUCTURE_OFFSET(ANTM_ANT_Version_Message_t, VersionData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Capabilities event                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_CAPABILITIES                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Capabilities_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          MaxChannels;
   unsigned int          MaxNetworks;
   unsigned int          StandardOptions;
   unsigned int          AdvancedOptions;
   unsigned int          AdvancedOptions2;
   unsigned int          Reserved;
} ANTM_Capabilities_Message_t;

#define ANTM_CAPABILITIES_MESSAGE_SIZE                         (sizeof(ANTM_Capabilities_Message_t))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Broadcast Data Packet event       */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_BROADCAST_DATA_PACKET           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Broadcast_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Broadcast_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Broadcast Data Packet Message*/
   /* given the number of actual data bytes.  This function accepts as  */
   /* it's input the total number individual data bytes are present     */
   /* starting from the Data member of the                              */
   /* ANTM_Broadcast_Data_Packet_Message_t structure and returns the    */
   /* total number of bytes required to hold the entire message.        */
#define ANTM_BROADCAST_DATA_PACKET_MESSAGE_SIZE(_x)            (STRUCTURE_OFFSET(ANTM_Broadcast_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Acknowledged Data Packet event    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_ACKNOWLEDGED_DATA_PACKET        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Acknowledged_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Acknowledged_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Acknowledged Data Packet     */
   /* Message given the number of actual data bytes.  This function     */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the Data member of the                      */
   /* ANTM_Acknowledged_Data_Packet_Message_t structure and returns the */
   /* total number of bytes required to hold the entire message.        */
#define ANTM_ACKNOWLEDGED_DATA_PACKET_MESSAGE_SIZE(_x)         (STRUCTURE_OFFSET(ANTM_Acknowledged_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Burst Data Packet event           */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_BURST_DATA_PACKET               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Burst_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          SequenceChannelNumber;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Burst_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Burst Data Packet Message    */
   /* given the number of actual data bytes.  This function accepts as  */
   /* it's input the total number individual data bytes are present     */
   /* starting from the Data member of the                              */
   /* ANTM_Burst_Data_Packet_Message_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
#define ANTM_BURST_DATA_PACKET_MESSAGE_SIZE(_x)                (STRUCTURE_OFFSET(ANTM_Burst_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Extended Broadcast Data Packet    */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             ANTM_MESSAGE_FUNCTION_EXTENDED_BROADCAST_DATA_PACKET  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Extended_Broadcast_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          DeviceNumber;
   unsigned int          DeviceType;
   unsigned int          TransmissionType;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Extended_Broadcast_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Extended Broadcast Data      */
   /* Packet Message given the number of actual data bytes.  This       */
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the Data member of the            */
   /* ANTM_Extended_Broadcast_Data_Packet_Message_t structure and       */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define ANTM_EXTENDED_BROADCAST_DATA_PACKET_MESSAGE_SIZE(_x)   (STRUCTURE_OFFSET(ANTM_Extended_Broadcast_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Extended Acknowledged Data Packet */
   /* event (asynchronously).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           ANTM_MESSAGE_FUNCTION_EXTENDED_ACKNOWLEDGED_DATA_PACKET */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Extended_Acknowledged_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          ChannelNumber;
   unsigned int          DeviceNumber;
   unsigned int          DeviceType;
   unsigned int          TransmissionType;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Extended_Acknowledged_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Extended Acknowledged Data   */
   /* Packet Message given the number of actual data bytes.  This       */
   /* function accepts as it's input the total number individual data   */
   /* bytes are present starting from the Data member of the            */
   /* ANTM_Extended_Acknowledged_Data_Packet_Message_t structure and    */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define ANTM_EXTENDED_ACKNOWLEDGED_DATA_PACKET_MESSAGE_SIZE(_x)   (STRUCTURE_OFFSET(ANTM_Extended_Acknowledged_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* ANT Manager Message for an ANT+ Extended Burst Data Packet event  */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           ANTM_MESSAGE_FUNCTION_EXTENDED_BURST_DATA_PACKET        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Extended_Burst_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          SequenceChannelNumber;
   unsigned int          DeviceNumber;
   unsigned int          DeviceType;
   unsigned int          TransmissionType;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Extended_Burst_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Extended Burst Data Packet   */
   /* Message given the number of actual data bytes.  This function     */
   /* accepts as it's input the total number individual data bytes are  */
   /* present starting from the Data member of the                      */
   /* ANTM_Extended_Burst_Data_Packet_Message_t structure and returns   */
   /* the total number of bytes required to hold the entire message.    */
#define ANTM_EXTENDED_BURST_DATA_PACKET_MESSAGE_SIZE(_x)       (STRUCTURE_OFFSET(ANTM_Extended_Burst_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition         */
   /* for a ANT Manager Message for an ANT+ Raw Data Packet event       */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           ANTM_MESSAGE_FUNCTION_RAW_DATA_PACKET                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagANTM_Raw_Data_Packet_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EventHandlerID;
   unsigned int          DataLength;
   Byte_t                Data[1];
} ANTM_Raw_Data_Packet_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Raw Data Packet Message given*/
   /* the number of actual data bytes.  This function accepts as it's   */
   /* input the total number individual data bytes are present starting */
   /* from the Data member of the ANTM_Raw_Data_Packet_Message_t        */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define ANTM_RAW_DATA_PACKET_MESSAGE_SIZE(_x)                  (STRUCTURE_OFFSET(ANTM_Raw_Data_Packet_Message_t, Data) + ((unsigned int)(_x)))

#endif

