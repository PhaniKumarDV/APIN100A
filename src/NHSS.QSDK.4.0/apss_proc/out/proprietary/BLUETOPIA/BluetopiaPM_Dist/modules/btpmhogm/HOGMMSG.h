/*****< hogmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HOGMMSG - Defined Interprocess Communication Messages for the Human       */
/*            Interface Device over GATT (HOGP) Manager for Stonestreet One   */
/*            Bluetopia Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HOGMMSGH__
#define __HOGMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHOGM.h"           /* HOGM Framework Prototypes/Constants.      */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HOGMType.h"            /* BTPM HOG Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Human Interface */
   /* Device over GATT (HID) Manager.                                   */
#define BTPM_MESSAGE_GROUP_HOGP_MANAGER                        0x00001109

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Human Interface  */
   /* Device over GATT (HOGP) Host Manager.                             */

   /* Human Interface Device over GATT (HOGP) Manager Commands.         */
#define HOGM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS              0x00001001
#define HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS           0x00001002

#define HOGM_MESSAGE_FUNCTION_REGISTER_HID_DATA                0x00001101
#define HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA             0x00001102

#define HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE                0x00002001
#define HOGM_MESSAGE_FUNCTION_SET_SUSPEND_MODE                 0x00002002
#define HOGM_MESSAGE_FUNCTION_GET_REPORT                       0x00002003
#define HOGM_MESSAGE_FUNCTION_SET_REPORT                       0x00002004

   /* Human Interface Device over GATT (HID) Manager Asynchronous       */
   /* Events.                                                           */
#define HOGM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED             0x00010002
#define HOGM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED          0x00010004

#define HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE_RESPONSE_EVENT 0x00011001
#define HOGM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT        0x00011002
#define HOGM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT        0x00011003

#define HOGM_MESSAGE_FUNCTION_DATA_INDICATION_EVENT            0x00020001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Human Interface Device  */
   /* Host (HID) Manager.                                               */

   /* Human Interface Device Host (HID) Manager Manager Command/Response*/
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to register for HOG Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Register_HID_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HOGM_Register_HID_Events_Request_t;

#define HOGM_REGISTER_HID_EVENTS_REQUEST_SIZE                  (sizeof(HOGM_Register_HID_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to register for HOG Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Register_HID_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          HOGEventsHandlerID;
} HOGM_Register_HID_Events_Response_t;

#define HOGM_REGISTER_HID_EVENTS_RESPONSE_SIZE                 (sizeof(HOGM_Register_HID_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to un-register for HOG Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Un_Register_HID_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HOGEventsHandlerID;
} HOGM_Un_Register_HID_Events_Request_t;

#define HOGM_UN_REGISTER_HID_EVENTS_REQUEST_SIZE               (sizeof(HOGM_Un_Register_HID_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to un-register for HOG Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Un_Register_HID_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HOGM_Un_Register_HID_Events_Response_t;

#define HOGM_UN_REGISTER_HID_EVENTS_RESPONSE_SIZE              (sizeof(HOGM_Un_Register_HID_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to register for HOG Manager Data events       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_REGISTER_HID_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Register_HID_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HOGM_Register_HID_Data_Events_Request_t;

#define HOGM_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE             (sizeof(HOGM_Register_HID_Data_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to register for HOG Manager Data events       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_REGISTER_HID_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Register_HID_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          HOGDataEventsHandlerID;
} HOGM_Register_HID_Data_Events_Response_t;

#define HOGM_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE            (sizeof(HOGM_Register_HID_Data_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to un-register for HOG Manager Data events    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Un_Register_HID_Data_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HOGDataEventsHandlerID;
} HOGM_Un_Register_HID_Data_Events_Request_t;

#define HOGM_UN_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE          (sizeof(HOGM_Un_Register_HID_Data_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to un-register for HOG Manager Data events    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_Un_Register_HID_Data_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HOGM_Un_Register_HID_Data_Events_Response_t;

#define HOGM_UN_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE         (sizeof(HOGM_Un_Register_HID_Data_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to set the Protocol Mode on a remote HID      */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Protocol_Mode_Request_t
{
   BTPM_Message_Header_t    MessageHeader;
   unsigned int             HOGManagerDataHandlerID;
   BD_ADDR_t                RemoteDeviceAddress;
   HOGM_HID_Protocol_Mode_t ProtocolMode;
} HOGM_HID_Set_Protocol_Mode_Request_t;

#define HOGM_HID_SET_PROTOCOL_MODE_REQUEST_SIZE                (sizeof(HOGM_HID_Set_Protocol_Mode_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to set the Protocol Mode on a remote HID      */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Protocol_Mode_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HOGM_HID_Set_Protocol_Mode_Response_t;

#define HOGM_HID_SET_PROTOCOL_MODE_RESPONSE_SIZE               (sizeof(HOGM_HID_Set_Protocol_Mode_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to set the Suspend Mode on a remote HID Device*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_SET_SUSPEND_MODE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Suspend_Mode_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HOGManagerDataHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Suspend;
} HOGM_HID_Set_Suspend_Mode_Request_t;

#define HOGM_HID_SET_SUSPEND_MODE_REQUEST_SIZE                 (sizeof(HOGM_HID_Set_Suspend_Mode_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to set the Suspend Mode on a remote HID Device*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_SET_SUSPEND_MODE                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Suspend_Mode_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HOGM_HID_Set_Suspend_Mode_Response_t;

#define HOGM_HID_SET_SUSPEND_MODE_RESPONSE_SIZE                (sizeof(HOGM_HID_Set_Suspend_Mode_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to do a Get Report procedure to a remote HID  */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_GET_REPORT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Get_Report_Request_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  HOGManagerDataHandlerID;
   BD_ADDR_t                     RemoteDeviceAddress;
   HOGM_HID_Report_Information_t ReportInformation;
} HOGM_HID_Get_Report_Request_t;

#define HOGM_HID_GET_REPORT_REQUEST_SIZE                (sizeof(HOGM_HID_Get_Report_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to do a Get Report procedure to a remote HID  */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_GET_REPORT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Get_Report_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} HOGM_HID_Get_Report_Response_t;

#define HOGM_HID_GET_REPORT_RESPONSE_SIZE               (sizeof(HOGM_HID_Get_Report_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to do a Set Report procedure to a remote HID  */
   /* Device (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_SET_REPORT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Report_Request_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  HOGManagerDataHandlerID;
   BD_ADDR_t                     RemoteDeviceAddress;
   HOGM_HID_Report_Information_t ReportInformation;
   Boolean_t                     ResponseExpected;
   unsigned int                  ReportDataLength;
   Byte_t                        ReportData[1];
} HOGM_HID_Set_Report_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Set Report Request Message   */
   /* given the number of actual report data bytes.  This function      */
   /* accepts as it's input the total number individual report data     */
   /* bytes are present starting from the ReportData member of the      */
   /* HOGM_HID_Set_Report_Request_t structure and returns the total     */
   /* number of bytes required to hold the entire message.              */
#define HOGM_HID_SET_REPORT_REQUEST_SIZE(_x)                   (STRUCTURE_OFFSET(HOGM_HID_Set_Report_Request_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message to do a Set Report procedure to a remote HID  */
   /* Device (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_SET_REPORT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Report_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} HOGM_HID_Set_Report_Response_t;

#define HOGM_HID_SET_REPORT_RESPONSE_SIZE               (sizeof(HOGM_HID_Set_Report_Response_t))

   /* HOG Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message that informs the client that a HID device is  */
   /* currently connected (asynchronously).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Device_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned long          SupportedFeatures;
   HOGM_HID_Information_t HIDInformation;
   unsigned int           ReportDescriptorLength;
   Byte_t                 ReportDescriptor[1];
} HOGM_HID_Device_Connected_Message_t;

   /* The following constants are used with the SupportedFeatures member*/
   /* of the HOGM_HID_Device_Connected_Message_t to denote various      */
   /* connection options.                                               */
#define HOGM_SUPPORTED_FEATURES_FLAGS_REMOTE_WAKEUP_CAPABLE    0x00000001
#define HOGM_SUPPORTED_FEATURES_FLAGS_NORMALLY_CONNECTABLE     0x00000002
#define HOGM_SUPPORTED_FEATURES_FLAGS_BOOT_KEYBOARD            0x00000004
#define HOGM_SUPPORTED_FEATURES_FLAGS_BOOT_MOUSE               0x00000008

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire HID Device Connected Message */
   /* given the number of actual report descriptor data bytes.  This    */
   /* function accepts as it's input the total number individual report */
   /* descriptor bytes are present starting from the ReportDescriptor   */
   /* member of the HOGM_HID_Device_Connected_Message_t structure and   */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
#define HOGM_HID_DEVICE_CONNECTED_MESSAGE_SIZE(_x)             (STRUCTURE_OFFSET(HOGM_HID_Device_Connected_Message_t, ReportDescriptor) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message that informs the client that a HID connection */
   /* is now disconnected (asynchronously).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HOGM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HOGM_HID_Device_Disconnected_Message_t;

#define HOGM_HID_DEVICE_DISCONNECTED_MESSAGE_SIZE              (sizeof(HOGM_HID_Device_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message that informs the client that a response to a  */
   /* previously sent Set Protocol Mode Request has been received       */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE_RESPONSE_EVENT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Protocol_Mode_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   Boolean_t             Success;
   Byte_t                AttributeErrorCode;
} HOGM_HID_Set_Protocol_Mode_Response_Message_t;

#define HOGM_HID_SET_PROTOCOL_MODE_RESPONSE_MESSAGE_SIZE       (sizeof(HOGM_HID_Set_Protocol_Mode_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message that informs the client that a response to a  */
   /* previously sent Get Report Request has been received              */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HOGM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Get_Report_Response_Message_t
{
   BTPM_Message_Header_t         MessageHeader;
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  TransactionID;
   Boolean_t                     Success;
   Byte_t                        AttributeErrorCode;
   HOGM_HID_Report_Information_t ReportInformation;
   unsigned int                  ReportDataLength;
   Byte_t                        ReportData[1];
} HOGM_HID_Get_Report_Response_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire HID Get Report Message given */
   /* the number of actual report data bytes.  This function accepts as */
   /* it's input the total number individual report bytes are present   */
   /* starting from the ReportData member of the                        */
   /* HOGM_HID_Get_Report_Response_Message_t structure and returns the  */
   /* total number of bytes required to hold the entire message.        */
#define HOGM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(_x)          (STRUCTURE_OFFSET(HOGM_HID_Get_Report_Response_Message_t, ReportData) + ((unsigned int)(_x)))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message that informs the client that a response to a  */
   /* previously sent Set Report Request has been received              */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HOGM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Set_Report_Response_Message_t
{
   BTPM_Message_Header_t         MessageHeader;
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  TransactionID;
   Boolean_t                     Success;
   Byte_t                        AttributeErrorCode;
   HOGM_HID_Report_Information_t ReportInformation;
} HOGM_HID_Set_Report_Response_Message_t;

#define HOGM_HID_SET_REPORT_RESPONSE_MESSAGE_SIZE              (sizeof(HOGM_HID_Set_Report_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HOG Manager Message that informs the client that data has been    */
   /* received from a remote HID Device (asynchronously).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HOGM_MESSAGE_FUNCTION_DATA_INDICATION_EVENT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHOGM_HID_Data_Indication_Message_t
{
   BTPM_Message_Header_t         MessageHeader;
   BD_ADDR_t                     RemoteDeviceAddress;
   HOGM_HID_Report_Information_t ReportInformation;
   unsigned int                  ReportDataLength;
   Byte_t                        ReportData[1];
} HOGM_HID_Data_Indication_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire HID Data Indication Message  */
   /* given the number of actual report data bytes.  This function      */
   /* accepts as it's input the total number individual report bytes are*/
   /* present starting from the ReportData member of the                */
   /* HOGM_HID_Data_Indication_Message_t structure and returns the total*/
   /* number of bytes required to hold the entire message.              */
#define HOGM_HID_DATA_INDICATION_MESSAGE_SIZE(_x)          (STRUCTURE_OFFSET(HOGM_HID_Data_Indication_Message_t, ReportData) + ((unsigned int)(_x)))

#endif

