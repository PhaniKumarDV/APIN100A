/*****< pbamapi.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PBAMAPI - Phone Book Access Profile API for Stonestreet One Bluetooth     */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __PBAMAPIH__
#define __PBAMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "PBAMMSG.h"             /* Phone Book Access IPC Message Definitions.*/

   /* The following structure is a container structure that is used to  */
   /* pass initialization information for the PBAP PCE role when it is  */
   /* initialized.                                                      */
typedef struct _tagPBAM_Client_Initialization_Data_t
{
   char          *ServiceName;
} PBAM_Client_Initialization_Data_t;


#define PBAM_CLIENT_INITIALIZATION_DATA_SIZE                   (sizeof(PBAM_Client_Initialization_Data_t))

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the MAP Module is initialized.    */
typedef struct _tagPBAM_Initialization_Info_t
{
   PBAM_Client_Initialization_Data_t *ClientInitializationData;
} PBAM_Initialization_Info_t;

#define PBAM_INITIALIZATION_INFO_SIZE                          (sizeof(PBAM_Intialization_Info_t))

   /* The following enumerated type represents the Phone Book Access    */
   /* manager event types that are dispatched by this module.           */
typedef enum
{
   petDisconnected,
   petConnectionStatus,
   petVCardData,
   petVCardListing,
   petPhoneBookSize,
   petPhoneBookSet,

   /* PSE role events.                                                  */
   petConnectionRequest,
   petConnected,
   petPullPhoneBook,
   petPullPhoneBookSize,
   petSetPhoneBook,
   petPullvCardListing,
   petPullvCardListingSize,
   petPullvCard,
   petAborted
} PBAM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petDisconnected event.  The     */
   /* following event is dispatched when a remote device disconnects    */
   /* from the local device role.  The RemoteDeviceAddress member       */
   /* specifies the Bluetooth device address of the remote device that  */
   /* disconnected from the profile.  The DisconnectReason member       */
   /* specifies whether or not the connection was disconnected via      */
   /* normal means or there was a service level connection establishment*/
   /* error.  After this event is received, the event callback          */
   /* (registered with PBAM_Connect_Remote_Device() function), will no  */
   /* longer receive events.                                            */
typedef struct _tagPBAM_Disconnected_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int DisconnectReason;
   
   /* This member only applies to the PSE role.                         */
   unsigned int ConnectionID;
} PBAM_Disconnected_Event_Data_t;

#define PBAM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(PBAM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petConnectionStatus event.  The */
   /* following event is dispatched when a client receives the          */
   /* connection response from a remote server which was previously     */
   /* attempted to be connected.  The RemoteDeviceAddress member        */
   /* specifies the remote device that was attempted to be connected to,*/
   /* and the ConnectionStatus member represents the connection status  */
   /* of the request.                                                   */
typedef struct _tagPBAM_Connection_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionStatus;
} PBAM_Connection_Status_Event_Data_t;

#define PBAM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(PBAM_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petVCardData event.  The        */
   /* following event is dispatched when the PBAM client receives a pull*/
   /* phonebook response from a remote PBAP Server.  The Staus member   */
   /* contains the response code included in the response packet.  The  */
   /* BufferSize and Buffer members contain the size and pointer        */
   /* (respectively) for the VCard data included in this pull phonebook */
   /* response.  The Final member indicates if this is the last block of*/
   /* data that will be sent in the pull phonebook transaction (Final = */
   /* TRUE if Server sent EndOfBody header).  The NewMissedCalls        */
   /* parameter is only valid if the "mch" PhoneBook was requested.  In */
   /* this case, this parameter contains the number of new missed calls */
   /* which have not been checked on the server.  Otherwise this        */
   /* parameter will default to zero.                                   */
typedef struct _tagPBAM_VCard_Event_Data_t
{
   BD_ADDR_t           RemoteDeviceAddress;
   unsigned int        Status;
   Boolean_t           Final;
   unsigned int        NewMissedCalls;
   PBAM_VCard_Format_t VCardFormat;
   unsigned int        BufferSize;
   Byte_t              Buffer[1];
} PBAM_VCard_Event_Data_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire vCard event data given the     */
   /* number of actual vCard bytes.  This function accepts as it's input*/
   /* the total number individual vCard data bytes are present starting */
   /* from the Buffer member of the PBAM_VCard_Event_Data_t structure   */
   /* and returns the total number of bytes required to hold the entire */
   /* message.                                                          */
#define PBAM_VCARD_EVENT_DATA_SIZE(_x)                         (STRUCTURE_OFFSET(PBAM_VCard_Event_Data_t, Buffer) + (unsigned int)(_x))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petVCardListing event. The      */
   /* following event is dispatched when the PBAM client receives a     */
   /* Pull vCard Listing Response from the remote PBAP server. The      */
   /* Status member contains the status of the request. The Final member*/
   /* contains a boolean indicating whether there is more data to be    */
   /* transmitted (Final = TRUE if Server sent EndOfBody header). The   */
   /* NewMissedCalls is valid only if the "mch" PhoneBook was requested.*/
   /* It contains the number of new missed calls which haven't been     */
   /* checked. BufferSize and Buffer conting the size and pointer to the*/
   /* actual listing data returned from the remote server.              */
typedef struct _tagPBAM_VCard_Listing_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Status;
   Boolean_t    Final;
   unsigned int NewMissedCalls;
   unsigned int BufferSize;
   Byte_t       Buffer[1];
} PBAM_VCard_Listing_Event_Data_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes     */
   /* that will be required to hold an entire vCard Listing event       */
   /* data given the number of actual vCard bytes. This function        */
   /* accepts as it's input the total number individual vCard data      */
   /* bytes are present starting from the Buffer member of the          */
   /* PBAM_VCard_Listing_Event_Data_t structure and returns the total   */
   /* number of bytes required to hold the entire message.              */
#define PBAM_VCARD_LISTING_EVENT_DATA_SIZE(_x)                 (STRUCTURE_OFFSET(PBAM_VCard_Listing_Event_Data_t, Buffer) + (unsigned int)(_x))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petPhoneBookSize.  The following*/
   /* event is dispatched when an PBAM client a response to from a      */
   /* remote PBAP server.  The Status indicates the success or failure  */
   /* of the request.  The PhoneBookSize parameter will contain the     */
   /* PhoneBook size returned by the remote PBAP server.  This size is  */
   /* may change from the time of the response to querying the device   */
   /* for data.                                                         */
typedef struct _tagPBAM_Phone_Book_Size_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Status;
   unsigned int PhoneBookSize;
} PBAM_Phone_Book_Size_Event_Data_t;

#define PBAM_PULL_PHONEBOOK_SIZE_EVENT_DATA_SIZE               (sizeof(PBAM_Phone_Book_Size_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petPhoneBookSet event.  The     */
   /* following PBAP Profile Event is dispatched when an PBAP client    */
   /* receives a set phonebook response from a remote PBAP Server.  The */
   /* RemoteDeviceAddress member specifies the local client that has    */
   /* received the response.  The Status member contains the response   */
   /* code included in the response packet.                             */
typedef struct _tagPBAM_Phone_Book_Set_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Status;
   unsigned int CurrentPathSize;
   char         CurrentPath[1];
} PBAM_Phone_Book_Set_Event_Data_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes     */
   /* that will be required to hold an entire vCard Listing event       */
   /* data given the number of actual vCard bytes. This function        */
   /* accepts as it's input the total number individual vCard data      */
   /* bytes are present starting from the Buffer member of the          */
   /* PBAM_VCard_Listing_Event_Data_t structure and returns the total   */
   /* number of bytes required to hold the entire message.              */
#define PBAM_PHONEBOOK_SET_EVENT_DATA_SIZE(_x)                 (STRUCTURE_OFFSET(PBAM_Phone_Book_Set_Event_Data_t, CurrentPath) + (unsigned int)(_x))

   /* This structure defines the data that is returned in a             */
   /* petConnectionRequest event. The ServerID member indicates         */
   /* the server instance that is being connected to.  The              */
   /* RemoteDeviceAddress member indicates the remote device's Bluetooth*/
   /* Address.  The ConnectionID member is the newly generated          */
   /* identifier for the new connection.                                */
typedef struct _tagPBAM_Connection_Request_Event_Data_t
{
   unsigned int ServerID;
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionID;
} PBAM_Connection_Request_Event_Data_t;

#define PBAM_CONNECTION_REQUEST_EVENT_DATA_SIZE                (sizeof(PBAM_Connection_Request_Event_Data_t))

   /* This structure defines the data that is returned in a petConnected*/
   /* event. The Server ID member indicates which server instance has   */
   /* been connected.  The RemoteDeviceAddress member indicates the     */
   /* Bluetooth Address of the remote device.  The ConnectionID member  */
   /* is the generated identifier for the new connection.               */
typedef struct _tagPBAM_Connected_Event_Data_t
{
   unsigned int ServerID;
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionID;
} PBAM_Connected_Event_Data_t;

#define PBAM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(PBAM_Connected_Event_Data_t))

   /* This structure defines the data that is returned in a             */
   /* petPullPhoneBook event.  The ConnectionID member indicates the    */
   /* connection on which the request is being made.  The ObjectName    */
   /* member indicates the Phonebook object to be pulled.  The FilterLow*/
   /* member contains the lower 32 bits of the filter field.  The       */
   /* FilterHigh member contains the higher 32 bits of the filter field.*/
   /* The vCardFormat indicates the desired vCard format in which to    */
   /* return the data.  The MaxListCount member indicates the maximum   */
   /* number of vCard entries to return.  The ListStartOffset member    */
   /* indicates an offset into the phonebook from which to begin the    */
   /* count.                                                            */
typedef struct _tagPBAM_Pull_Phone_Book_Event_Data_t
{
   unsigned int         ConnectionID;
   char                *ObjectName;
   DWord_t              FilterLow;
   DWord_t              FilterHigh;
   PBAM_VCard_Format_t  vCardFormat;
   unsigned int         MaxListCount;
   unsigned int         ListStartOffset;
} PBAM_Pull_Phone_Book_Event_Data_t;

#define PBAM_PULL_PHONE_BOOK_EVENT_DATA_SIZE                   (sizeof(PBAM_Pull_Phone_Book_Event_Data_t))

   /* This structure defines the data that is returned in a             */
   /* petPullPhoneBookSize event.  The ConnectionID member indicates the*/
   /* connection on which the request is being made.  The ObjectName    */
   /* indicates the Phonebook object for which to pull the size.        */
typedef struct _tagPBAM_Pull_Phone_Book_Size_Event_Data_t
{
   unsigned int  ConnectionID;
   char         *ObjectName;
} PBAM_Pull_Phone_Book_Size_Event_Data_t;

#define PBAM_PULL_PHONE_BOOK_SIZE_EVENT_DATA_SIZE              (sizeof(PBAM_Pull_Phone_Book_Size_Event_Data_t))

   /* This structure defines the data that is returned in a             */
   /* petSetPhoneBook event.  The ConnectionID member indicates the     */
   /* connection on which the request is being made.  The PathOption    */
   /* member indicates what type of navigation is being requested.  The */
   /* ObjectName member indicates the folder to switch to.              */
typedef struct _tagPBAM_Set_Phone_Book_Event_Data_t
{
   unsigned int            ConnectionID;
   PBAM_Set_Path_Option_t  PathOption;
   char                   *ObjectName;
} PBAM_Set_Phone_Book_Event_Data_t;

#define PBAM_SET_PHONE_BOOK_EVENT_DATA_SIZE                    (sizeof(PBAM_Set_Phone_Book_Event_Data_t))

   /* This structure defines the data that is returned in a             */
   /* petPullvCardListing event.  The ConnectionID member indicates the */
   /* connection on which the request is being made.  The ObjectName    */
   /* member indicates the folder for which the vCard listing is being  */
   /* requested. If this value is NULL, then the current folder should  */
   /* be used.  The ListOrder indicates the order in which the vCards   */
   /* should be listed.  The SearchAttribute member indicates what      */
   /* attribute the SearchValue applies to.  The SearchValue member     */
   /* indicates a value to search. If this value is NULL, no search was */
   /* requested.  The MaxListCount member indicates the maximum number  */
   /* of vCard entries to return.  The ListStartOffset member indicates */
   /* an offset into the phonebook from which to begin the count.       */
typedef struct _tagPBAM_Pull_vCard_Listing_Event_Data_t
{
   unsigned int             ConnectionID;
   char                    *ObjectName;
   PBAM_List_Order_t        ListOrder;
   PBAM_Search_Attribute_t  SearchAttribute;
   char                    *SearchValue;
   unsigned int             MaxListCount;
   unsigned int             ListStartOffset;
} PBAM_Pull_vCard_Listing_Event_Data_t;

#define PBAM_PULL_VCARD_LISTING_EVENT_DATA_SIZE                (sizeof(PBAM_Pull_vCard_Listing_Event_Data_t))

   /* This structure defines the data that is returned in a             */
   /* petPullvCardListingSize event.  The ConnectionID member indicates */
   /* the connection on which the request is being made.  The ObjectName*/
   /* indicates the Folder for which to pull the listing size. If this  */
   /* value is NULL, the the current directory should be used.          */
typedef struct _tagPBAM_Pull_vCard_Listing_Size_Event_Data_t
{
   unsigned int  ConnectionID;
   char         *ObjectName;
} PBAM_Pull_vCard_Listing_Size_Event_Data_t;

#define PBAM_PULL_VCARD_LISTING_SIZE_EVENT_DATA_SIZE           (sizeof(PBAM_Pull_vCard_Listing_Size_Event_Data_t))

   /* This structure defines the data that is returned in a petPullvCard*/
   /* event.  The ConnectionID member indicates the connection on which */
   /* the request is being made.  The ObjectName member indicates the   */
   /* name of the vCard to be pulled.  The FilterLow member contains    */
   /* the lower 32 bits of the filter field.  The FilterHigh member     */
   /* contains the higher 32 bits of the filter field.  The Format      */
   /* member indicates the vCard Format in which to return the data.    */
typedef struct _tagPBAM_Pull_vCard_Event_Data_t
{
   unsigned int         ConnectionID;
   char                *ObjectName;
   DWord_t              FilterLow;
   DWord_t              FilterHigh;
   PBAM_VCard_Format_t  Format;
} PBAM_Pull_vCard_Event_Data_t;

#define PBAM_PULL_VCARD_EVENT_DATA_SIZE                        (sizeof(PBAM_Pull_vCard_Event_Data_t))

   /* This structure defines the data that is returned int a petAborted */
   /* event.  The ConnectionID member indicates the connection on which */
   /* the request is being made.                                        */
typedef struct _tagPBAM_Aborted_Event_Data_t
{
   unsigned int ConnectionID;
} PBAM_Aborted_Event_Data_t;

#define PBAM_ABORTED_EVENT_DATA_SIZE                           (sizeof(PBAM_Aborted_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Phone Book Access Manager Event (and Event Data) of a Phone Book  */
   /* Access Manager Event.                                             */
typedef struct _tagPBAM_Event_Data_t
{
   PBAM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      PBAM_Disconnected_Event_Data_t            DisconnectedEventData;
      PBAM_Connection_Status_Event_Data_t       ConnectionStatusEventData;
      PBAM_VCard_Event_Data_t                   VCardEventData;
      PBAM_VCard_Listing_Event_Data_t           VCardListingEventData;
      PBAM_Phone_Book_Size_Event_Data_t         PhoneBookSizeEventData;
      PBAM_Phone_Book_Set_Event_Data_t          PhoneBookSetEventData;

      /* PSE role event data structures.                                */
      PBAM_Connection_Request_Event_Data_t      ConnectionRequestEventData;
      PBAM_Connected_Event_Data_t               ConnectedEventData;
      PBAM_Pull_Phone_Book_Event_Data_t         PullPhoneBookEventData;
      PBAM_Pull_Phone_Book_Size_Event_Data_t    PullPhoneBookSizeEventData;
      PBAM_Set_Phone_Book_Event_Data_t          SetPhoneBookEventData;
      PBAM_Pull_vCard_Listing_Event_Data_t      PullvCardListingEventData;
      PBAM_Pull_vCard_Listing_Size_Event_Data_t PullvCardListingSizeEventData;
      PBAM_Pull_vCard_Event_Data_t              PullvCardEventData;
      PBAM_Aborted_Event_Data_t                 AbortedEventData;
   } EventData;
} PBAM_Event_Data_t;

#define PBAM_EVENT_DATA_SIZE                                   (sizeof(PBAM_Event_Data_t))

   /* The following declared type represents the prototype function for */
   /* an event and data callback.  This function will be called whenever*/
   /* the Phone Book Access Manager dispatches an event.  This function */
   /* passes to the caller the Phone Book Access Manager event and the  */
   /* callback parameter that was specified when this callback was      */
   /* installed.  The caller is free to use the contents of the         */
   /* EventData ONLY in the context of this callback.  If the caller    */
   /* requires the data for a longer period of time, then the callback  */
   /* function MUST copy the data into another data buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the thread*/
   /* context of a thread that the user does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another message will not be   */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by receiving other Events.  A deadlock */
   /*          WILL occur because NO event callbacks will be issued     */
   /*          while this function is currently outstanding.            */
typedef void (BTPSAPI *PBAM_Event_Callback_t)(PBAM_Event_Data_t *EventData, void *CallbackParameter);

   /* Phone Book Access Module Installation/Support Functions.          */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Phone Book Access Manager module.  */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when the Platform       */
   /* Manager is initialized (or shut down).                            */
void BTPSAPI PBAM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PBAM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Phone Book Access Manager Connection Management Functions.        */

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Phone Book Access device.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function also accepts the       */
   /* connection information for the remote device (address and server  */
   /* port).  This function accepts the connection flags to apply to    */
   /* control how the connection is made regarding encryption and/or    */
   /* authentication.                                                   */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Phone Book Access Manager connection status Event (if    */
   /*          specified).                                              */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, PBAM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Connect_Remote_Device_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, PBAM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function exists to close an active Phone Book Access*/
   /* connection that was previously opened by a successful call to     */
   /* PBAM_Connect_Remote_Device() function.  This function accepts the */
   /* RemoteDeviceAddress.  This function returns zero if successful, or*/
   /* a negative return value if there was an error.                    */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Disconnect_Device_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding PBAM profile client request.  This function accepts as*/
   /* input the remote device address of the device to abort the current*/
   /* operation.  This function returns zero if successful, or a        */
   /* negative return error code if there was an error.                 */
   /* * NOTE * There can only be one outstanding PBAM request active at */
   /*          any one time.  Because of this, another PBAM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the PBAM_Abort() function) or the    */
   /*          current request is completed (this is signified by       */
   /*          receiving a confirmation event in the PBAM event callback*/
   /*          that was registered when the PBAM port was opened).      */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Abort(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Abort_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function generates a PBAP Pull PhoneBook Request to */
   /* the specified remote PBAP server.  The RemoteDeviceAddress        */
   /* parameter specifies the device connection to perform the pull     */
   /* request The PhoneBookNamePath parameter contains the name/path of */
   /* the phonebook being requested by this pull phone book operation.  */
   /* The FilterLow parameter contains the lower 32 bits of the 64-bit  */
   /* filter attribute.  The FilterHigh parameter contains the higher 32*/
   /* bits of the 64-bit filter attribute.  The VCardFormat parameter is*/
   /* an enumeration which specifies the VCard format requested in this */
   /* Pull PhoneBook request.  If pfDefault is specified then the format*/
   /* will not be included in the request.  The MaxListCount parameter  */
   /* is an unsigned integer that specifies the maximum number of       */
   /* entries the client can handle.  A value of 65535 means that the   */
   /* number of entries is not restricted.  A MaxListCount of ZERO (0)  */
   /* indicates that this is a request for the number of used indexes in*/
   /* the PhoneBook specified by the PhoneBookNamePath parameter.  The  */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this PullPhoneBook.  This function returns zero if      */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP profile server successfully */
   /*          executed the request.                                    */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAM   */
   /*          request cannot be issued until either the current request*/
   /*          is aborted (by calling the PBAM_Abort() function) or the */
   /*          current request is completed (this is signified by       */
   /*          receiving an petPBAData event, with Final TRUE, in the   */
   /*          callback that was registered when the PBAM port was      */
   /*          opened).                                                 */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Pull_Phone_Book(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, Word_t MaxListCount, Word_t ListStartOffset);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Pull_Phone_Book_t)(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat, Word_t MaxListCount, Word_t ListStartOffset);
#endif

   /* The following function generates a PBAP Pull PhoneBook Size       */
   /* Request to the specified remote PBAP server requesting the size of*/
   /* the phonebook.  The RemoteDeviceAddress parameter specifies the   */
   /* connect for the local PBAP Client (returned from a successful call*/
   /* to the PBAM_Connect_Remote_Device() This size returned in the     */
   /* event, petPhoneBookSize, may change from the time of the response */
   /* to querying the device for data.                                  */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPhoneBookSize event in the PBAM event     */
   /*          callback that was registered when the PBAM port was      */
   /*          opened).                                                 */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Pull_Phone_Book_Size(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Pull_Phone_Book_Size_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function generates a PBAP Set Phone Book Request to */
   /* the specified remote PBAP Server.  The RemoteDeviceAddress        */
   /* parameter specifies the connected device for the request.  The    */
   /* PathOption parameter contains an enumerated value that indicates  */
   /* the type of path change to request.  The FolderName parameter     */
   /* contains the folder name to include with this Set PhoneBook       */
   /* request.  This value can be NULL if no name is required for the   */
   /* selected PathOption.  See the PBAP specification for more         */
   /* information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Set_Phone_Book(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Set_Phone_Book_t)(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName);
#endif

   /* The following function generates a PBAP Pull vCard Listing Request*/
   /* to the specified remote PBAP Server. The RemoteDeviceAddress      */
   /* parameter specifies the connected device for the request. The     */
   /* PhonebookPath Parameter specifies the name of the phonebook to    */
   /* pull the listing from. The ListOrder parameter is an enumerated   */
   /* type that determines the optionally requested order of listing.   */
   /* Using 'loDefault' will prevent the field from being added. The    */
   /* SearchAttribute parameter is an enumerated type that specifies    */
   /* the requested attribute to be used as a search filter. The        */
   /* SearchValue contains an optional ASCII string that contains the   */
   /* requested search value. If this is NULL, it will be excluded. The */
   /* MaxListCount is an unsigned integer that represents the maximum   */
   /* number of list entries to be returned. The ListStartOffset        */
   /* parameter specifies the index requested. This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Pull_vCard_Listing(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Pull_vCard_Listing_t)(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset);
#endif

   /* The following function generates a PBAP Pull vCard Entry Request  */
   /* to the specified remote PBAP Server. The RemoteDeviceAddress      */
   /* Parameter specifies the connected device for the request. The     */
   /* vCardName parameter is an ASCII string representing the name of   */
   /* the vCard to be pulled in the request. The FilterLow parameter    */
   /* contains the lower 32 bits of the 64-bit filter attribute. The    */
   /* FilterHigh parameter contains the higher 32 bits of the 64-bit    */
   /* filter attribute. The Format parameter is an enumeration which    */
   /* specifies the format of the vCard requested. This function returns*/
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Pull_vCard(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Pull_vCard_t)(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat);
#endif

   /* The following function wraps PBAP Set Phone Book Requests in order*/
   /* to supply an absolute path to change to. The RemoteDeviceAddress  */
   /* parameter specifies the connected device for the request. The     */
   /* AbsolutePath parameter is an ASCII string containing the path to  */
   /* set the phone book to. This function returns zero if successful   */
   /* and a negative return error code if there was and error.          */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP server successfully processed the command.  The     */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * If there is an error while processing the series of      */
   /*          requests, a petPhoneBookSetEvent will be sent containing */
   /*          the path before the failure occurred. This will can be   */
   /*          assumed to be the current path.                          */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is aborted (by calling the PBAM_Abort() function)*/
   /*          or the current request is completed (this is signified by*/
   /*          receiving a petPBASetPhoneBookStatus event in the PBAM   */
   /*          event callback that was registered when the PBAM port was*/
   /*          opened).                                                 */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Set_Phone_Book_Absolute(BD_ADDR_t RemoteDeviceAddress, char *AbsolutePath);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Set_Phone_Book_Absolute_t)(BD_ADDR_t RemoteDeviceAddress, char *AbsolutePath);
#endif


   /* PSE Role API Functions.                                           */


   /* Register a PBAP Server Port. The ServerPort parameter specifies   */
   /* the RFCOMM port number on which to open the server. The           */
   /* SupportedRepositories parameter is a bit mask of the supported    */
   /* local contact database types. The IncomingConnectionFlags         */
   /* parameter is a bitmask that determine how to handle incoming      */
   /* connection requests to the server port. The ServiceName parameter */
   /* is the service name to insert into the SDP record for the         */
   /* server. The EventCallback parameter is the callback function      */
   /* that will receive asynchronous events for this server. The        */
   /* CallbackParameter will be passed to the EventCallback when events */
   /* are dispatched. On success, this function returns a positive,     */
   /* non-zero value representing the ServerID for the newly opened     */
   /* server. On failure, this function returns a negative error code.  */
   /* * NOTE * Supplying a ServerPort of 0 will cause this function to  */
   /*          automatically pick an available port number.             */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Register_Server(unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName, PBAM_Event_Callback_t EventCallback, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Register_Server_t)(unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName, PBAM_Event_Callback_t EventCallback, void *CallbackParameter);
#endif

   /* Unregisters a previously opened PBAP server port. The ServerID    */
   /* parameter is the ID of the server returned from a successful call */
   /* to PBAM_Register_Server(). This fuction returns zero if successful*/
   /* or a negative return error code if there was an error.            */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Un_Register_Server(unsigned int ServerID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Un_Register_Server_t)(unsigned int ServerID);
#endif

   /* Respond to an outstanding connection request to the local         */
   /* server. The ConnectionID is the indentifier of the connection     */
   /* request returned in a petConnectionRequest event. The Accept      */
   /* parameter indicates whether the connection should be accepted or  */
   /* rejected. This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Connection_Request_Response(unsigned int ConnectionID, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Connection_Request_Response_t)(unsigned int ConnectionID, Boolean_t Accept);
#endif

   /* Close an active connection to a local PBAP Server instance. The   */
   /* ConnectionID parameter is the identifier of the connection        */
   /* returned in a petConnected event. This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Close_Server_Connection(unsigned int ConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Close_Server_Connection_t)(unsigned int ConnectionID);
#endif

   /* Submits a response to a received etPullPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the active connection */
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes. If the request  */
   /* event indicated the 'mch' phonebook, the NewMissedCalls parameter */
   /* should be a pointer to the value of the number of missed calls    */
   /* since the last 'mch' pull request. If it is not an 'mch' request, */
   /* this parameter should be set to NULL. The BufferSize parameter    */
   /* indicates the amount of data in the buffer to be sent. The Buffer */
   /* parameter is a pointer to the phone book data to send. The Final  */
   /* parameter should be set to FALSE if there is more data to be sent */
   /* after this buffer or TRUE if there is no more data. This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Send_Phone_Book(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Send_Phone_Book_t)(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);
#endif

   /* Submits a response to a received etPullPhoneBookSize event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode          */
   /* parameter is one of the defined PBAM Response Status codes. The   */
   /* PhonebookSize parameter indicates the number of entries in the    */
   /* requested phone book. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Send_Phone_Book_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int PhoneBookSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Send_Phone_Book_Size_t)(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int PhoneBookSize);
#endif

   /* Submits a response to a received petSetPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes. This function   */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Set_Phone_Book_Response(unsigned int ConnectionID, unsigned int ResponseStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Set_Phone_Book_Response_t)(unsigned int ConnectionID, unsigned int ResponseStatusCode);
#endif

   /* Submits a response to a received petPullvCardListing event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes.  If the request */
   /* event indicated the 'mch' phonebook, the NewMissedCalls parameter */
   /* should be a pointer to the value of the number of missed calls    */
   /* since the last 'mch' pull request. If it is not an 'mch' request, */
   /* this parameter should be set to NULL. The BufferSize parameter    */
   /* indicates the amount of data in the buffer to be sent. The Buffer */
   /* parameter is a pointer to the vCardListing data to send. The Final*/
   /* parameter should be set to FALSE if there is more data to be sent */
   /* after this buffer or TRUE if there is no more data. This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Send_vCard_Listing(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Send_vCard_Listing_t)(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);
#endif

   /* Submits a response to a received petPullvCardListingSize          */
   /* event. The ConnectionID parameter is the identifier of the        */
   /* activer connection returned in a petConnected event. The          */
   /* ResponseStatusCode parameter is one of the defined PBAM Response  */
   /* Status codes. The vCardListingSize parameter indicates the number */
   /* of vCard entries in the current/specfied folder. This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Send_vCard_Listing_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int vCardListingSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Send_vCard_Listing_Size_t)(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int vCardListingSize);
#endif

   /* Submits a response to a received petPullvCard event. The          */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined PBAM Response Status codes.  The BufferSize */
   /* parameter indicates the amount of data in the buffer to be        */
   /* sent. The Buffer parameter is a pointer to the vCard data to      */
   /* send. The Final parameter should be set to FALSE if there is more */
   /* data to be sent after this buffer or TRUE if there is no more     */
   /* data. This function returns zero if successful and a negative     */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI PBAM_Send_vCard(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PBAM_Send_vCard_t)(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);
#endif

#endif

