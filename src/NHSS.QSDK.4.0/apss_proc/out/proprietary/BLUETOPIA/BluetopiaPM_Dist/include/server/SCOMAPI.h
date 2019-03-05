/*****< scomapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SCOMAPI - Local SCO Manager API for Stonestreet One Bluetooth Protocol    */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/07/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SCOMAPIH__
#define __SCOMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SCOMMSG.h"             /* BTPM SCO Manager Message Formats.         */

   /* The following enumerated type represents the SCO Manager Event    */
   /* Types that are dispatched by this module.                         */
typedef enum
{
   setServerConnectionOpen,
   setConnectionClose,
   setRemoteConnectionOpenStatus
} SCOM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setServerConnectionOpen event.  */
typedef struct _tagSCOM_Server_Connection_Open_Event_Data_t
{
   unsigned int ConnectionID;
   BD_ADDR_t    RemoteDeviceAddress;
} SCOM_Server_Connection_Open_Event_Data_t;

#define SCOM_SERVER_CONNECTION_OPEN_EVENT_DATA_SIZE            (sizeof(SCOM_Server_Connection_Open_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setConnectionClose event.       */
typedef struct _tagSCOM_Connection_Close_Event_Data_t
{
   unsigned int ConnectionID;
} SCOM_Connection_Close_Event_Data_t;

#define SCOM_CONNECTION_CLOSE_EVENT_DATA_SIZE                  (sizeof(SCOM_Connection_Close_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setRemoteConnectionOpenStatus   */
   /* event.                                                            */
typedef struct _tagSCOM_Remote_Connection_Open_Status_Event_Data_t
{
   unsigned int ConnectionID;
   int          Status;
} SCOM_Remote_Connection_Open_Status_Event_Data_t;

#define SCOM_REMOTE_CONNECTION_OPEN_STATUS_EVENT_DATA_SIZE     (sizeof(SCOM_Remote_Connection_Open_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* SCO Event (and Event Data) of a SCO Manager Event.                */
typedef struct _tagSCOM_Event_Data_t
{
   SCOM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      SCOM_Server_Connection_Open_Event_Data_t        ServerConnectionOpenEventData;
      SCOM_Connection_Close_Event_Data_t              ConnectionCloseEventData;
      SCOM_Remote_Connection_Open_Status_Event_Data_t RemoteConnectionOpenStatusEventData;
   } EventData;
} SCOM_Event_Data_t;

#define SCOM_EVENT_DATA_SIZE                                   (sizeof(SCOM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the SCO */
   /* Manager dispatches an event (and the client has registered for    */
   /* events).  This function passes to the caller the SCO Manager Event*/
   /* and the Callback Parameter that was specified when this Callback  */
   /* was installed.  The caller is free to use the contents of the     */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *SCOM_Event_Callback_t)(SCOM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a Local SCO Manager Server.  This function    */
   /* accepts a flag that specifies whether or not the server will      */
   /* accept incoming connections, and if so, the device that is allowed*/
   /* to connect.  The final two parameters specify the Event Callback  */
   /* and Callback parameter (to receive events related to the          */
   /* registered server).  This function returns a positive, non-zero,  */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * A successful return value represents the Connection ID   */
   /*          that can be used with various functions in this module   */
   /*          to specify this local server connection.                 */
   /* * NOTE * If a specific is not required, and ANY device is to be   */
   /*          specified for the Remote Device (second parameter), then */
   /*          a NULL device address should be specified (all zeros).   */
BTPSAPI_DECLARATION int BTPSAPI SCOM_RegisterServerConnection(Boolean_t EnableConnection, BD_ADDR_t RemoteDevice, SCOM_Event_Callback_t EventCallback, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_RegisterServerConnection_t)(Boolean_t EnableConnection, BD_ADDR_t RemoteDevice, SCOM_Event_Callback_t EventCallback, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Server Connection. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI SCOM_UnRegisterServerConnection(unsigned int ConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_UnRegisterServerConnection_t)(unsigned int ConnectionID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to enable/disable incoming connections for a specified    */
   /* Server Connection.  This function accepts the Server connection   */
   /* for which to enable/disable incoming connections followed by a    */
   /* flag that specifies whether or not the server will accept incoming*/
   /* connections, and if so, the device that is allowed to connect.    */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If a specific device is not required, and ANY device is  */
   /*          to be specified for the Remote Device (second parameter),*/
   /*          then a NULL device address should be specified (all      */
   /*          zeros).                                                  */
   /* * NOTE * The connection cannot be disabled for an individual      */
   /*          device.  If the second parameter is specified as FALSE   */
   /*          than ALL incoming connections are rejected and the final */
   /*          parameter is ignored.                                    */
BTPSAPI_DECLARATION int BTPSAPI SCOM_EnableServerConnection(unsigned int ConnectionID, Boolean_t EnableConnection, BD_ADDR_t RemoteDevice);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_EnableServerConnection_t)(unsigned int ConnectionID, Boolean_t EnableConnection, BD_ADDR_t RemoteDevice);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to open remote SCO connection.  This function returns a   */
   /* positive, non-zero, value if successful, or a negative return     */
   /* error code if there was an error.  If this function is successful,*/
   /* the value that is returned represents the SCO Manager Connection  */
   /* ID of the connection.                                             */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Connection Result event.                     */
BTPSAPI_DECLARATION int BTPSAPI SCOM_OpenRemoteConnection(BD_ADDR_t RemoteDevice, SCOM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_OpenRemoteConnection_t)(BD_ADDR_t RemoteDevice, SCOM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an active connection for the specified SCO       */
   /* Connection (either server or remote connection).  This function   */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * This function will NOT Un-Register a local server        */
   /*          connection, it will just disconnect any currently        */
   /*          connected remote device.                                 */
BTPSAPI_DECLARATION int BTPSAPI SCOM_CloseConnection(unsigned int ConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_CloseConnection_t)(unsigned int ConnectionID);
#endif

   /* The following function is provided to allow local modules the     */
   /* ability to inform the SCO Manager to ignore incoming connections  */
   /* from the specified device.  This is useful if a particular module */
   /* is tracking SCO connections itself (for example Headset or        */
   /* Handsfree modules).  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI SCOM_AddConnectionToIgnoreList(BD_ADDR_t RemoteDevice);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_AddConnectionToIgnoreList_t)(BD_ADDR_t RemoteDevice);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to remove a (previously added) device from the ignore     */
   /* list.  This function accepts as input the device to remove from   */
   /* the list.  This function returns zero if successful or a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * If the device specified is a NULL BD_ADDR (all zero's)   */
   /*          then this function will clear the entire ignore list.    */
BTPSAPI_DECLARATION int BTPSAPI SCOM_DeleteConnectionFromIgnoreList(BD_ADDR_t RemoteDevice);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SCOM_DeleteConnectionFromIgnoreList_t)(BD_ADDR_t RemoteDevice);
#endif

#endif
