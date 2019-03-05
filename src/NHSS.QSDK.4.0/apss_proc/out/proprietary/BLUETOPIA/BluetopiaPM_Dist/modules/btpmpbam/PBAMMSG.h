/*****< pbammsg.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PBAMMSG - Defined Interprocess Communication Messages for the Phone Book  */
/*            Access Manager (PBAM) for Stonestreet One Bluetopia Protocol    */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __PBAMMSGH__
#define __PBAMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTPBAM.h"           /* Phone Book Access Prototypes/Constants.   */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "PBAMType.h"            /* BTPM PBA Manager Type Definitions.        */

   /* The following message group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Phone Book      */
   /* Access (PBA) Manager.                                             */
#define BTPM_MESSAGE_GROUP_PHONE_BOOK_ACCESS_MANAGER           0x00001003

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message functions that are valid for the Phone Book Access*/
   /* (PBA) manager.                                                    */

   /* Phone Book Access Client (PBA-PCE) Manager Commands.              */
#define PBAM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE            0x00001002
#define PBAM_MESSAGE_FUNCTION_DISCONNECT_DEVICE                0x00001003
#define PBAM_MESSAGE_FUNCTION_ABORT                            0x00001004
#define PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK                  0x00001005
#define PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE             0x00001006
#define PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK                   0x00001007
#define PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING               0x00001008
#define PBAM_MESSAGE_FUNCTION_PULL_VCARD                       0x00001009
#define PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_ABSOLUTE          0x00001010

   /* Phone Book Access Server (PBA-PSE) Managaer Commands.             */
#define PBAM_MESSAGE_FUNCTION_REGISTER_SERVER                  0x00002001
#define PBAM_MESSAGE_FUNCTION_UN_REGISTER_SERVER               0x00002002
#define PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE      0x00002003
#define PBAM_MESSAGE_FUNCTION_CLOSE_SERVER_CONNECTION          0x00002004
#define PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK                  0x00002005
#define PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE             0x00002006
#define PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_RESPONSE          0x00002007
#define PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING               0x00002008
#define PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING_SIZE          0x00002009
#define PBAM_MESSAGE_FUNCTION_SEND_VCARD                       0x0000200A

   /* Phone Book Access Client (PBA-PCE) Manager Asynchronous Events.   */
#define PBAM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS         0x00010003
#define PBAM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED              0x00010004
#define PBAM_MESSAGE_FUNCTION_VCARD_DATA                       0x00010005
#define PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SIZE                  0x00010006
#define PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET                   0x00010007
#define PBAM_MESSAGE_FUNCTION_VCARD_LISTING_DATA               0x00010008

   /* Phone Book Access Server (PBA-PSE) Manager Asynchronous Events.   */
#define PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST               0x00020001
#define PBAM_MESSAGE_FUNCTION_CONNECTED                        0x00020002
#define PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_EVENT            0x00020003
#define PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE_EVENT       0x00020004
#define PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK_EVENT             0x00020005
#define PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING_EVENT         0x00020006
#define PBAM_MESSAGE_FUNCTION_PULL_VCARD_LISTING_SIZE_EVENT    0x00020007
#define PBAM_MESSAGE_FUNCTION_PULL_VCARD_EVENT                 0x00020008
#define PBAM_MESSAGE_FUNCTION_ABORTED                          0x00020009

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to connect to a remote device   */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          RemoteServerPort;
   unsigned long         ConnectionFlags;
   BD_ADDR_t             RemoteDeviceAddress;
} PBAM_Connect_Remote_Device_Request_t;

#define PBAM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(PBAM_Connect_Remote_Device_Request_t))

   /* The following constants are used with the ConnectFlags parameter  */
   /* of the PBAM_Connect_Remote_Device_Request_t message.              */
#define PBAM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION   0x00000001
#define PBAM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION       0x00000002

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to connect to a remote device   */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Connect_Remote_Device_Response_t;

#define PBAM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(PBAM_Connect_Remote_Device_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to disconnect a currently     */
   /* connected device (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_DISCONNECT_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Disconnect_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} PBAM_Disconnect_Device_Request_t;

#define PBAM_DISCONNECT_DEVICE_REQUEST_SIZE                    (sizeof(PBAM_Disconnect_Device_Request_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to disconnect a currently     */
   /* connected device (Response).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_DISCONNECT_DEVICE               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Disconnect_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Disconnect_Device_Response_t;

#define PBAM_DISCONNECT_DEVICE_RESPONSE_SIZE                   (sizeof(PBAM_Disconnect_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to abort a currently active     */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_ABORT                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Abort_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} PBAM_Abort_Request_t;

#define PBAM_ABORT_REQUEST_SIZE                                (sizeof(PBAM_Abort_Request_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to abort a currently active     */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_ABORT                           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Abort_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Abort_Response_t;

#define PBAM_ABORT_REQUEST_RESPONSE_SIZE                       (sizeof(PBAM_Abort_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager request message to pull a phone book    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_Phone_Book_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               FilterLow;
   DWord_t               FilterHigh;
   PBAM_VCard_Format_t   VCardFormat;
   Word_t                MaxListCount;
   Word_t                ListStartOffset;
   unsigned int          PhoneBookNamePathSize;
   char                  PhoneBookNamePath[1];
} PBAM_Pull_Phone_Book_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the PhoneBookNamePath and returns the total number of bytes   */
   /* required to hold the entire message.                              */
#define PBAM_PULL_PHONE_BOOK_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(PBAM_Pull_Phone_Book_Request_t, PhoneBookNamePath) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager request message to pull a phone book    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_Phone_Book_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Pull_Phone_Book_Response_t;

#define PBAM_PULL_PHONE_BOOK_RESPONSE_SIZE                     (sizeof(PBAM_Pull_Phone_Book_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to query a phone book size    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_Phone_Book_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} PBAM_Pull_Phone_Book_Size_Request_t;

#define PBAM_PULL_PHONE_BOOK_SIZE_REQUEST_SIZE                 (sizeof(PBAM_Pull_Phone_Book_Size_Request_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to query a phone book size    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_PULL_PHONE_BOOK_SIZE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_Phone_Book_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Pull_Phone_Book_Size_Response_t;

#define PBAM_PULL_PHONE_BOOK_SIZE_RESPONSE_SIZE                (sizeof(PBAM_Pull_Phone_Book_Size_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to set the current phone book */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   PBAM_Set_Path_Option_t PathOption;
   unsigned int           FolderNameSize;
   char                   FolderName[1];
} PBAM_Set_Phone_Book_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the FolderName and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_SET_PHONE_BOOK_REQUEST_SIZE(_x)                   (STRUCTURE_OFFSET(PBAM_Set_Phone_Book_Request_t, FolderName) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to set the current phone book */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Set_Phone_Book_Response_t;

#define PBAM_SET_PHONE_BOOK_SIZE_RESPONSE_SIZE                 (sizeof(PBAM_Set_Phone_Book_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to pull a vCard listing       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD_LISTING                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Listing_Request_t
{
   BTPM_Message_Header_t   MessageHeader;
   BD_ADDR_t               RemoteDeviceAddress;
   PBAM_List_Order_t       ListOrder;
   PBAM_Search_Attribute_t SearchAttribute;
   Word_t                  MaxListCount;
   Word_t                  ListStartOffset;
   unsigned int            PhonebookPathSize;
   unsigned int            SearchValueSize;
   Byte_t                  VariableData[1];
} PBAM_Pull_vCard_Listing_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* variable data. This function accepts as it's input the total      */
   /* number bytes for the PhonebookPath and SearchValue and returns the*/
   /* total number of bytes required to hold the entire message.        */
#define PBAM_PULL_VCARD_LISTING_REQUEST_SIZE(_x,_y)            (STRUCTURE_OFFSET(PBAM_Pull_vCard_Listing_Request_t, VariableData) + ((unsigned int)(_x)) + ((unsigned int)(_y)))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to pull a vCard listing       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD_LISTING                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Listing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} PBAM_Pull_vCard_Listing_Response_t;

#define PBAM_PULL_VCARD_LISTING_RESPONSE_SIZE                  (sizeof(PBAM_Pull_vCard_Listing_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to pull a vCard (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD                               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               FilterLow;
   DWord_t               FilterHigh;
   PBAM_VCard_Format_t   VCardFormat;
   unsigned int          VCardNameSize;
   char                  VCardName[1];
} PBAM_Pull_vCard_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the VCardName and returns the total number of bytes required  */
   /* to hold the entire message.                                       */
#define PBAM_PULL_VCARD_REQUEST_SIZE(_x)                       (STRUCTURE_OFFSET(PBAM_Pull_vCard_Request_t, VCardName) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to pull a vCard (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD                               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int Status;
} PBAM_Pull_vCard_Response_t;

#define PBAM_PULL_VCARD_RESPONSE_SIZE                          (sizeof(PBAM_Pull_vCard_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to set the current phone book to*/
   /* an absolute path (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Absolute_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          PathSize;
   char                  Path[1];
} PBAM_Set_Phone_Book_Absolute_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the Path and returns the total number of bytes required to    */
   /* hold the entire message.                                          */
#define PBAM_SET_PHONE_BOOK_ABSOLUTE_REQUEST_SIZE(_x)          (STRUCTURE_OFFSET(PBAM_Set_Phone_Book_Absolute_Request_t, Path) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to set the current phone book to*/
   /* an absolute path (Request).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SET_PHONE_BOOK                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Absolute_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          Status;
} PBAM_Set_Phone_Book_Absolute_Response_t;

#define PBAM_SET_PHONE_BOOK_ABSOLUTE_RESPONSE_SIZE             (sizeof(PBAM_Set_Phone_Book_Absolute_Response_t))


   /* PBAP-PSE Management Commands.                                     */


   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to register a PBAP server port  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_REGISTER_SERVER                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Register_Server_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerPort;
   unsigned int          SupportedRepositories;
   unsigned long         IncomingConnectionFlags;
   unsigned int          ServiceNameLength;
   Byte_t                ServiceName[1];
} PBAM_Register_Server_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the ServiceName and returns the total number of bytes required*/
   /* to hold the entire message.                                       */
#define PBAM_REGISTER_SERVER_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(PBAM_Register_Server_Request_t, ServiceName) + ((unsigned int)(_x)))

   /* The following constants are used with the SupportedRepositories   */
   /* parameter of the PBAM_Register_Server_Request_t message.          */
#define PBAM_SUPPORTED_REPOSITORIES_LOCAL_PHONEBOOK            0x00000001
#define PBAM_SUPPORTED_REPOSITORIES_SIM_CARD_PHONEBOOK         0x00000002

   /* The following constants are used with the IncomingConnectionFlags */
   /* parameter of the PBAM_Register_Server_Request_t message.          */
#define PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION   0x00000001
#define PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION  0x00000002
#define PBAM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION      0x00000004

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to register a PBAP server port  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_REGISTER_SERVER                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Register_Server_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Register_Server_Response_t;

#define PBAM_REGISTER_SERVER_RESPONSE_SIZE                     (sizeof(PBAM_Register_Server_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to un register a server port    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_UN_REGISTER_SERVER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Un_Register_Server_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
} PBAM_Un_Register_Server_Request_t;

#define PBAM_UN_REGISTER_SERVER_REQUEST_SIZE                   (sizeof(PBAM_Un_Register_Server_Request_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to un register a server port    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_UN_REGISTER_SERVER              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Un_Register_Server_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Un_Register_Server_Response_t;

#define PBAM_UN_REGISTER_SERVER_RESPONSE_SIZE                  (sizeof(PBAM_Un_Register_Server_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to respond to a connection    */
   /* request (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   Boolean_t             Accept;
} PBAM_Connection_Request_Response_Request_t;

#define PBAM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE          (sizeof(PBAM_Connection_Request_Response_Request_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to respond to a connection    */
   /* request (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Connection_Request_Response_Response_t;

#define PBAM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE         (sizeof(PBAM_Connection_Request_Response_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to close and active server    */
   /* connection (Request).                                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_CLOSE_SERVER_CONNECTION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct PBAM_Close_Server_Connection_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
} PBAM_Close_Server_Connection_Request_t;

#define PBAM_CLOSE_SERVER_CONNECTION_REQUEST_SIZE              (sizeof(PBAM_Close_Server_Connection_Request_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to close and active server    */
   /* connection (Response).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_CLOSE_SERVER_CONNECTION         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct PBAM_Close_Server_Connection_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Close_Server_Connection_Response_t;

#define PBAM_CLOSE_SERVER_CONNECTION_RESPONSE_SIZE             (sizeof(PBAM_Close_Server_Connection_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to send phone book data       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_Phone_Book_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   unsigned int          ResponseStatusCode;
   Boolean_t             IncludeMissedCalls;
   Byte_t                NewMissedCalls;
   Boolean_t             Final;
   unsigned int          BufferSize;
   Byte_t                Buffer[1];
} PBAM_Send_Phone_Book_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* buffer. This function accepts as it's input the total number bytes*/
   /* for the BufferSize and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_SEND_PHONE_BOOK_REQUEST_SIZE(_x)                  (STRUCTURE_OFFSET(PBAM_Send_Phone_Book_Request_t, Buffer) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to send phone book data       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_Phone_Book_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Send_Phone_Book_Response_t;

#define PBAM_SEND_PHONE_BOOK_RESPONSE_SIZE                     (sizeof(PBAM_Send_Phone_Book_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to send a phone book size     */
   /* response (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_Phone_Book_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   unsigned int          ResponseStatusCode;
   unsigned int          PhoneBookSize;
} PBAM_Send_Phone_Book_Size_Request_t;

#define PBAM_SEND_PHONE_BOOK_SIZE_REQUEST_SIZE                 (sizeof(PBAM_Send_Phone_Book_Size_Request_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to send a phone book size     */
   /* response (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_Phone_Book_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Send_Phone_Book_Size_Response_t;

#define PBAM_SEND_PHONE_BOOK_SIZE_RESPONSE_SIZE                (sizeof(PBAM_Send_Phone_Book_Size_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to send a set phonebook response*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   unsigned int          ResponseStatusCode;
} PBAM_Set_Phone_Book_Response_Request_t;

#define PBAM_SET_PHONE_BOOK_RESPONSE_REQUEST_SIZE              (sizeof(PBAM_Set_Phone_Book_Response_Request_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to send a set phonebook response*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_PHONE_BOOK_SIZE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Set_Phone_Book_Response_Response_t;

#define PBAM_SET_PHONE_BOOK_RESPONSE_RESPONSE_SIZE             (sizeof(PBAM_Set_Phone_Book_Response_Response_t))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to send vcard listing data    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_vCard_Listing_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   unsigned int          ResponseStatusCode;
   Boolean_t             IncludeMissedCalls;
   Byte_t                NewMissedCalls;
   Boolean_t             Final;
   unsigned int          BufferSize;
   Byte_t                Buffer[1];
} PBAM_Send_vCard_Listing_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* buffer. This function accepts as it's input the total number bytes*/
   /* for the BufferSize and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_SEND_VCARD_LISTING_REQUEST_SIZE(_x)               (STRUCTURE_OFFSET(PBAM_Send_vCard_Listing_Request_t, Buffer) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for     */
   /* a Phone Book Access Manager message to send vcard listing data    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_vCard_Listing_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Send_vCard_Listing_Response_t;

#define PBAM_SEND_VCARD_LISTING_RESPONSE_SIZE                  (sizeof(PBAM_Send_vCard_Listing_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to send a vCard listing size    */
   /* response (Request).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING_SIZE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_vCard_Listing_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   unsigned int          ResponseStatusCode;
   unsigned int          vCardListingSize;
} PBAM_Send_vCard_Listing_Size_Request_t;

#define PBAM_SEND_VCARD_LISTING_SIZE_REQUEST_SIZE              (sizeof(PBAM_Send_vCard_Listing_Size_Request_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to send a vCard listing size    */
   /* response (Response).                                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_VCARD_LISTING_SIZE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_vCard_Listing_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Send_vCard_Listing_Size_Response_t;

#define PBAM_SEND_VCARD_LISTING_SIZE_RESPONSE_SIZE             (sizeof(PBAM_Send_vCard_Listing_Size_Response_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to send vCard data (Request).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_VCARD                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_vCard_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionID;
   unsigned int          ResponseStatusCode;
   Boolean_t             Final;
   unsigned int          BufferSize;
   Byte_t                Buffer[1];
} PBAM_Send_vCard_Request_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* buffer. This function accepts as it's input the total number bytes*/
   /* for the BufferSize and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_SEND_VCARD_REQUEST_SIZE(_x)                       (STRUCTURE_OFFSET(PBAM_Send_vCard_Request_t, Buffer) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message to send vCard data (Response).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_SEND_VCARD                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Send_vCard_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} PBAM_Send_vCard_Response_t;

#define PBAM_SEND_VCARD_RESPONSE_SIZE                          (sizeof(PBAM_Send_vCard_Response_t))

   /* PBAP-PCE Aysnchronous message definitions.                        */

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of the  */
   /* specified connection status (asynchronously).                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_DEVICE_CONNECTION_STATUS        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Device_Connection_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ConnectionStatus;
   BD_ADDR_t             RemoteDeviceAddress;
} PBAM_Device_Connection_Message_t;

#define PBAM_DEVICE_CONNECTION_MESSAGE_SIZE                    (sizeof(PBAM_Device_Connection_Message_t))

   /* The following constants are used with the ConnectionStatus member */
   /* of the PBAM_Device_Connection_Message_t message to describe the   */
   /* actual connection result status.                                  */
#define PBAM_DEVICE_CONNECTION_STATUS_SUCCESS                  0x00000000
#define PBAM_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT          0x00000001
#define PBAM_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED          0x00000002
#define PBAM_DEVICE_CONNECTION_STATUS_FAILURE_SECURITY         0x00000003
#define PBAM_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF 0x00000004
#define PBAM_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN          0x00000005

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client that a  */
   /* Phone Book Access connection is now disconnected (asynchronously).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_DEVICE_DISCONNECTED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Device_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          DisconnectReason;

   /* PSE Role only.                                                    */
   unsigned int          ServerID;
   unsigned int          ConnectionID;
} PBAM_Device_Disconnected_Message_t;

#define PBAM_DEVICE_DISCONNECTED_MESSAGE_SIZE                  (sizeof(PBAM_Device_Disconnected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that contains retrieved vCard   */
   /* data (asynchronously).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_VCARD_DATA                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_VCard_Data_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          Status;
   Boolean_t             Final;
   unsigned int          NewMissedCalls;
   PBAM_VCard_Format_t   Format;
   unsigned int          BufferSize;
   Byte_t                Buffer[1];
} PBAM_VCard_Data_Message_t;

#define PBAM_VCARD_DATA_MESSAGE_SIZE(_x)                       (STRUCTURE_OFFSET(PBAM_VCard_Data_Message_t, Buffer) + ((unsigned int)(_x)))

   /* The following constants are used with the Status member of the    */
   /* PBAM_VCard_Data_Message_t message to describe the actual          */
   /* connection result status.                                         */
#define PBAM_VCARD_DATA_STATUS_SUCCESS                         0x00000000
#define PBAM_VCARD_DATA_STATUS_ABORT                           0x00000001
#define PBAM_VCARD_DATA_STATUS_FAILURE_UNKNOWN                 0x00000002

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that contains retrieved vCard   */
   /* listing data (asynchronously).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_VCARD_DATA                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_VCard_Listing_Data_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Final;
   unsigned int          Status;
   unsigned int          NewMissedCalls;
   unsigned int          BufferSize;
   Byte_t                Buffer[1];
} PBAM_VCard_Listing_Data_Message_t;

#define PBAM_VCARD_LISTING_DATA_MESSAGE_SIZE(_x)               (STRUCTURE_OFFSET(PBAM_VCard_Listing_Data_Message_t, Buffer) + ((unsigned int)(_x)))

#define PBAM_VCARD_LISTING_STATUS_SUCCESS                      0x00000000
#define PBAM_VCARD_LISTING_STATUS_ABORT                        0x00000001
#define PBAM_VCARD_LISTING_STATUS_FAILURE_UNKNOWN              0x00000002

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of the  */
   /* phone book size status (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SIZE                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Phone_Book_Size_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          Status;
   unsigned int          PhoneBookSize;
} PBAM_Phone_Book_Size_Message_t;

#define PBAM_PHONE_BOOK_SIZE_MESSAGE_SIZE                      (sizeof(PBAM_Phone_Book_Size_Message_t))

   /* The following constants are used with the Status member of the    */
   /* PBAM_PhoneBookSizeEventData_t message to describe the actual      */
   /* connection result status.                                         */
#define PBAM_PHONE_BOOK_SIZE_STATUS_SUCCESS                    0x00000000
#define PBAM_PHONE_BOOK_SIZE_STATUS_FAILURE_UNKNOWN            0x00000001

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of the  */
   /* phone book set status (asynchronously).                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_FUNCTION_PHONE_BOOK_SET                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Phone_Book_Set_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          Status;
   unsigned int          CurrentPathSize;
   Byte_t                CurrentPath[1];
} PBAM_Phone_Book_Set_Message_t;

#define PBAM_PHONE_BOOK_SET_MESSAGE_SIZE(_x)                   (STRUCTURE_OFFSET(PBAM_Phone_Book_Set_Message_t, CurrentPath) + (unsigned int)(_x))

   /* The following constants are used with the Status member of the    */
   /* PBAM_Phone_Book_Set_Message_t message to describe the actual set  */
   /* phone book status.                                                */
#define PBAM_PHONE_BOOK_SET_STATUS_SUCCESS                     0x00000000
#define PBAM_PHONE_BOOK_SET_STATUS_FAILURE_UNKNOWN             0x00000001


   /* PBAP-PSE Aysnchronous message definitions.                        */


   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* connection request (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_CONNECTION_REQUEST                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Connection_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionID;
} PBAM_Connection_Request_Message_t;

#define PBAM_CONNECTION_REQUEST_MESSAGE_SIZE                   (sizeof(PBAM_Connection_Request_Message_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* newly connected device (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_CONNECTED                                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ConnectionID;
} PBAM_Connected_Message_t;

#define PBAM_CONNECTED_MESSAGE_SIZE                            (sizeof(PBAM_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* pull phone book request (asynchronously).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_PHONE_BOOK_EVENT                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_Phone_Book_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ConnectionID;
   DWord_t               FilterLow;
   DWord_t               FilterHigh;
   PBAM_VCard_Format_t   vCardFormat;
   unsigned int          MaxListCount;
   unsigned int          ListStartOffset;
   unsigned int          ObjectNameLength;
   char                  ObjectName[1];
} PBAM_Pull_Phone_Book_Message_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the ObjectName and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_PULL_PHONE_BOOK_EVENT_MESSAGE_SIZE(_x)            (STRUCTURE_OFFSET(PBAM_Pull_Phone_Book_Message_t, ObjectName) + (unsigned int)(_x))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* pull phone book size request (asynchronously).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_PHONE_BOOK_SIZE_EVENT               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_Phone_Book_Size_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ConnectionID;
   unsigned int          ObjectNameLength;
   char                  ObjectName[1];
} PBAM_Pull_Phone_Book_Size_Message_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the ObjectName and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_PULL_PHONE_BOOK_SIZE_EVENT_MESSAGE_SIZE(_x)       (STRUCTURE_OFFSET(PBAM_Pull_Phone_Book_Size_Message_t, ObjectName) + (unsigned int)(_x))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a set*/
   /* phone book request (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_SET_PHONE_BOOK_EVENT                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Set_Phone_Book_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           ServerID;
   unsigned int           ConnectionID;
   PBAM_Set_Path_Option_t PathOption;
   unsigned int           ObjectNameLength;
   char                   ObjectName[1];
} PBAM_Set_Phone_Book_Message_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the ObjectName and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_SET_PHONE_BOOK_EVENT_MESSAGE_SIZE(_x)             (STRUCTURE_OFFSET(PBAM_Set_Phone_Book_Message_t, ObjectName) + (unsigned int)(_x))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* pull vCard listing request (asynchronously).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD_LISTING_EVENT                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Listing_Message_t
{
   BTPM_Message_Header_t   MessageHeader;
   unsigned int            ServerID;
   unsigned int            ConnectionID;
   PBAM_List_Order_t       ListOrder;
   PBAM_Search_Attribute_t SearchAttribute;
   unsigned int            MaxListCount;
   unsigned int            ListStartOffset;
   unsigned int            ObjectNameLength;
   unsigned int            SearchValueLength;
   Byte_t                  VariableData[1];
} PBAM_Pull_vCard_Listing_Message_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* strings. This function accepts as it's input the total number     */
   /* bytes for the ObjectName and SearchValue and returns the total    */
   /* number of bytes required to hold the entire message.              */
#define PBAM_PULL_VCARD_LISTING_EVENT_MESSAGE_SIZE(_x,_y)         (STRUCTURE_OFFSET(PBAM_Pull_vCard_Listing_Message_t, VariableData) + (unsigned int)(_x) + (unsigned int)(_y))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* pull vCard Listing size request (asynchronously).                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD_LISTING_SIZE_EVENT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Listing_Size_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ConnectionID;
   unsigned int          ObjectNameLength;
   char                  ObjectName[1];
} PBAM_Pull_vCard_Listing_Size_Message_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the ObjectName and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_PULL_VCARD_LISTING_SIZE_EVENT_MESSAGE_SIZE(_x)    (STRUCTURE_OFFSET(PBAM_Pull_vCard_Listing_Size_Message_t, ObjectName) + (unsigned int)(_x))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of a    */
   /* pull vCard request (asynchronously).                              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_PULL_VCARD_EVENT                         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Pull_vCard_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ConnectionID;
   DWord_t               FilterLow;
   DWord_t               FilterHigh;
   PBAM_VCard_Format_t   vCardFormat;
   unsigned int          ObjectNameLength;
   char                  ObjectName[1];
} PBAM_Pull_vCard_Message_t;

   /* The following macro is provided to allow the programmer a means   */
   /* of quickly determining the total number of bytes that will be     */
   /* required to hold an entire data message given the size of the     */
   /* string. This function accepts as it's input the total number bytes*/
   /* for the ObjectName and returns the total number of bytes required */
   /* to hold the entire message.                                       */
#define PBAM_PULL_VCARD_EVENT_MESSAGE_SIZE(_x)                 (STRUCTURE_OFFSET(PBAM_Pull_vCard_Message_t, ObjectName) + (unsigned int)(_x))

   /* The following structure represents the message definition for a   */
   /* Phone Book Access Manager message that informs the client of an   */
   /* aborted operation (asynchronously).                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             PBAM_MESSAGE_ABORTED                                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagPBAM_Aborted_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerID;
   unsigned int          ConnectionID;
} PBAM_Aborted_Message_t;

#define PBAM_ABORTED_MESSAGE_SIZE                              (sizeof(PBAM_Aborted_Message_t))

#endif
