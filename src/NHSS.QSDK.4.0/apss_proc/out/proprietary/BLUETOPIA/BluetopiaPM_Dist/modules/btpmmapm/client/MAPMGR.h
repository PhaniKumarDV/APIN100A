/*****< mapmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAPMGR - Message Access Manager Implementation for Stonestreet One        */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/24/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __MAPMGRH__
#define __MAPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTMAPM.h"           /* MAP Framework Prototypes/Constants.       */

   /* Message Access Module Installation/Support Functions.             */

   /* Initializes the platform manager client Message Access Manager    */
   /* implementation.  This function returns zero if successful, or a   */
   /* negative integer if there was an error.                           */
int _MAPM_Initialize(void);

   /* This function sets the pm client Initialized global flag to false.*/
   /* The other methods will fail until _MAPM_Initialize has been       */
   /* called.                                                           */
void _MAPM_Cleanup(void);

   /* Message Access Profile (MAP) Manager (MAPM) Common Functions.     */

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming MAP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _MAPM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Accept);

   /* The following function is provided to allow a mechanism for local */
   /* to register a MAP Server (MSE) on a specified RFCOMM Server Port. */
   /* This function accepts as it's parameter's the RFCOMM Server Port  */
   /* to register the server on, followed by the incoming connection    */
   /* flags to apply to incoming connections.  The third and fourth     */
   /* parameters specify the required MAP Information (MAP Server       */
   /* Instance ID - must be unique on the device, followed by the       */
   /* supported MAP Message Types).  The final two parameters specify   */
   /* the MAP Manager Event Callback function and Callback parameter    */
   /* which will be used when MAP Manager events need to be dispatched  */
   /* for the specified MAP Server.  This function returns zero if      */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * This function is only applicable to MAP Servers.  It will*/
   /*          not allow the ability to register a MAP Notification     */
   /*          Server (Notification Servers are handled internal to the */
   /*          MAP Manager module).                                     */
int _MAPM_Register_Server(unsigned int ServerPort, unsigned long ServerFlags, unsigned int InstanceID, unsigned long SupportedMessageTypes);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered MAP server (registered via a  */
   /* successful call the the MAP_Register_Server() function).  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _MAPM_Un_Register_Server(unsigned int InstanceID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to Register an SDP Service Record for a previously        */
   /* registered MAP Server.  This function returns a positive,         */
   /* non-zero, value if successful, or a negative return error code if */
   /* there was an error.  If this function is successful, the value    */
   /* that is returned represents the SDP Service Record Handle of the  */
   /* Service Record that was added to the SDP Database.  The           */
   /* ServiceName parameter is a pointer to a NULL terminated UTF-8     */
   /* encoded string.                                                   */
long _MAPM_Register_Service_Record(unsigned int InstanceID, char *ServiceName);

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered SDP Service Record.*/
   /* This function accepts the Instance ID of the MAP Server that is to*/
   /* have the Service Record Removed.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _MAPM_Un_Register_Service_Record(unsigned int InstanceID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the details of MAP services offered by a remote  */
   /* Message Access Server device. This function accepts the remote    */
   /* device address of the device whose SDP records will be parsed     */
   /* and the buffer which will hold the parsed service details. This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
   /* * NOTE * When this function is successful, the provided buffer    */
   /*          will be populated with the parsed service details. This  */
   /*          buffer MUST be passed to                                 */
   /*          MAPM_Free_Parsed_Message_Access_Service_Info() in order  */
   /*          to release any resources that were allocated during the  */
   /*          query process.                                           */
int _MAPM_Parse_Remote_Message_Access_Services(BD_ADDR_t RemoteDeviceAddress, MAPM_Parsed_Message_Access_Service_Info_t *ServiceInfo);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Message Access Server device.  The */
   /* first parameter to this function specifies the connection type to */
   /* make (either Notification or Message Access).  The                */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The InstancedID    */
   /* member *MUST* specify the Remote Instance ID of the remote MAP    */
   /* server that is to be connected with.  The ConnectionFlags         */
   /* parameter specifies whether authentication or encryption should be*/
   /* used to create this connection.  This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
int _MAPM_Connect_Remote_Device(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned int InstanceID, unsigned long ConnectionFlags);

   /* The following function exists to close an active Message Access   */
   /* connection that was previously opened by a successful call to     */
   /* MAPM_Connect_Server() function or by a metConnectServer.          */
   /* This function accepts the RemoteDeviceAddress. The                */
   /* InstanceID parameter specifies which server instance to use.      */
   /* The ConnectionType parameter indicates what type of connection    */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
int _MAPM_Disconnect(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding MAPM profile client or notification client request.   */
   /* This function accepts as input the connection type of the remote  */
   /* connection, followed by the remote device address of the device to*/
   /* abort the current operation, followed by the InstanceID parameter.*/
   /* Together these parameters specify which connection is to have the */
   /* Abort issued.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
int _MAPM_Abort(MAPM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

   /* Message Access Client (MCE) Functions.                            */

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current folder.  The first parameter is the  */
   /* Bluetooth address of the device whose connection we are querying. */
   /* The InstanceID parameter specifies which server instance on the   */
   /* remote device to use.  The second parameter is the size of the    */
   /* Buffer that is available to store the current path.  The final    */
   /* parameter is the buffer to copy the path in to.  This function    */
   /* returns a positive (or zero) value representing the total length  */
   /* of the path string (including the NULL character) if successful   */
   /* and a negative return error code if there was an error.           */
   /* * NOTE * If the current path is at root, then the Buffer will     */
   /*          contain an empty string and the length will be zero.     */
   /* * NOTE * If the supplied buffer was not large enough to hold the  */
   /*          returned size, it will still be NULL-terminated but will */
   /*          not contain the complete path.                           */
int _MAPM_Query_Current_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int BufferSize, char *Buffer);

   /* The following function is provided to allow a mechanism for local */
   /* modules to create and enable a Notification server for a specified*/
   /* connection.  The RemoteDeviceAddress parameter specifies what     */
   /* connected device this server should be associated with.  The      */
   /* InstanceID parameter specifies which server instance on the remote*/
   /* device to use.  The ServerPort parameter is the local RFCOMM port */
   /* on which to open the server.  The Callback Function and Parameter */
   /* will be called for all events related to this notification server.*/
int _MAPM_Enable_Notifications(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Boolean_t Enabled);

   /* The following function generates a MAP Set Folder Request to the  */
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The PathOption*/
   /* parameter contains an enumerated value that indicates the type of */
   /* path change to request.  The FolderName parameter contains the    */
   /* folder name to include with this Set Folder request.  This value  */
   /* can be NULL if no name is required for the selected PathOption.   */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Set_Folder(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, MAP_Set_Folder_Option_t PathOption, char *FolderName);

   /* The following function is provided to allow a mechanism for local */
   /* modules set the folder to an absolute path.  This function        */
   /* generates a sequence of MAP Set Folder Requests, navigating to the*/
   /* supplied path.  The RemoteDeviceAddress is the address of the     */
   /* remote server.  The InstanceID parameter specifies which server   */
   /* instance on the remote device to use.  The FolderName parameter is*/
   /* a string containing containg a path from the root to the desired  */
   /* folder (i.e. telecom/msg/inbox).  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Set_Folder_Absolute(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName);

   /* The following function generates a MAP Get Folder Listing Request */
   /* to the specified remote MAP Server. The RemoteDeviceAddress       */
   /* is the address of the remote server. The InstanceID parameter     */
   /* specifies which server instance on the remote device to use.      */
   /* The MaxListCount is positive, non-zero integer representing the   */
   /* maximum amount of folder entries to return. The ListStartOffset   */
   /* signifies an offset to request. This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Get_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, Word_t MaxListCount, Word_t ListStartOffset);

   /* The following function is provided to allow a mechanism for local */
   /* modules to simply request the size of folder listing. It accepts  */
   /* as a parameter the address of the remote server. The InstanceID   */
   /* parameter specifies which server instance on the remote device    */
   /* to use. This function returns zero if successful and a negative   */
   /* return error code if there was an error.                          */
int _MAPM_Get_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

   /* The following function generates a MAP Get Message Listing Request*/
   /* to the specified remote MAP Server.  The RemoteDeviceAddress is   */
   /* the address of the remote server.  The InstanceID parameter       */
   /* specifies which server instance on the remote device to use.  The */
   /* FolderName parameter specifies the direct sub-directory to pull   */
   /* the listing from.  If this is NULL, the listing will be from the  */
   /* current directory.  The MaxListCount is a positive, non-zero      */
   /* integer representing the maximum amount of folder entries to      */
   /* return.  The ListStartOffset signifies an offset to request.  The */
   /* ListingInfo parameter is an optional parameter which, if          */
   /* specified, points to a structure which contains a set of filters  */
   /* to use when pulling the listing.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Get_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo);

   /* The following function is provided to allow a mechanism for local */
   /* modules to simply request the size of a message listing.  It      */
   /* accepts as a parameter the address of the remote server and the   */
   /* folder name from which to pull the listing.  A value of NULL      */
   /* indicates the current folder should be used.  The InstanceID      */
   /* parameter specifies which server instance on the remote device to */
   /* use.  The ListingInfo parameter is an optional parameter which, if*/
   /* specified, points to a structure which contains a set of filters  */
   /* to use when pulling the listing.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
   /* return error code if there was an error.                          */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
int _MAPM_Get_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, MAP_Message_Listing_Info_t *ListingInfo);

   /* The following function generates a MAP Get Message Request to     */
   /* the specified remote MAP Server. The RemoteDeviceAddress is       */
   /* the address of the remote server. The InstanceID parameter        */
   /* specifies which server instance on the remote device to use. The  */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.      */
   /* The Attachment parameter indicates whether any attachments to     */
   /* the message should be included in the response. The CharSet and   */
   /* FractionalType parameters specify the format of the response. This*/
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
int _MAPM_Get_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType);

   /* The following function generates a MAP Set Message Status Request */
   /* to the specified remote MAP Server.  The RemoteDeviceAddress is   */
   /* the address of the remote server.  The InstanceID parameter       */
   /* specifies which server instance on the remote device to use.  The */
   /* MessageHandle is a 16 byte NULL terminated string containing      */
   /* Unicode hexadecimal characters which identifies the message.  The */
   /* StatusIndicator signifies which indicator to be set.  The         */
   /* StatusValue is the value to set.  This function returns zero if   */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Set_Message_Status(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue);

   /* The following function generates a MAP Push Message Request to the*/
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  The FolderName*/
   /* parameter specifies the direct sub-directory to pull the listing  */
   /* from.  If this is NULL, the listing will be from the current      */
   /* directory.  The Transparent parameter indicates whether a copy    */
   /* should be placed in the sent folder.  The Retry parameter         */
   /* indicates if any retries should be attempted if sending fails.    */
   /* The CharSet specifies the format of the message.  The DataLength  */
   /* parameter indicates the length of the supplied data.  The         */
   /* DataBuffer parameter is a pointer to the data to send.  The Final */
   /* parameter indicates if the buffer supplied is all of the data to  */
   /* be sent.  This function returns zero if successful and a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * The FolderName buffer (if specified) should point to a   */
   /*          NULL terminated, UTF-8 encoded ASCII string.             */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Push_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, char *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

   /* The following function generates a MAP Update Inbox Request to the*/
   /* specified remote MAP Server.  The RemoteDeviceAddress is the      */
   /* address of the remote server.  The InstanceID parameter specifies */
   /* which server instance on the remote device to use.  This function */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _MAPM_Update_Inbox(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID);

   /* Message Access Client (MCE) Notification Functions.               */

   /* The following function generates a MAP Send Event Request to the  */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The DataLength */
   /* Parameter specifies the length of the data.  The Buffer contains  */
   /* the data to be sent.  This function returns zero if successful and*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Send_Notification(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int DataLength, Byte_t *EventData, Boolean_t Final);

   /* Message Access Server (MSE) Functions.                            */

   /* The following function generates a MAP Set Notification           */
   /* Registration Response to the specified remote MAP Client.  The    */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode Parameter is the OBEX Response   */
   /* Code to send with the response.  This function returns zero if    */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Enable_Notifications_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

   /* The following function generates a MAP Set Folder Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
int _MAPM_Set_Folder_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

   /* The following function generates a MAP Folder Listing Response to */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The DataLength parameter specifies the length of the   */
   /* data.  The Buffer contains the data to be sent.  This function    */
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Send_Folder_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, unsigned int FolderListingLength, Byte_t *FolderListing, Boolean_t Final);

   /* The following function generates a Folder Listing Response to the */
   /* specified remote MAP Client.  The RemoteDeviceAddress is the      */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The Size of the size of the listing to return.  This   */
   /* function returns zero if successful and a negative return error   */
   /* code if there was an error.                                       */
int _MAPM_Send_Folder_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t FolderCount);

   /* The following function generates a MAP Message Listing Response to*/
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The MessageCount supplies the number of messages in the*/
   /* listing.  The NewMessage parameter indicates if there are new     */
   /* messages since the last pull.  CurrentTime indicates the time of  */
   /* the response.  The DataLength parameter specifies the length of   */
   /* the data.  The Buffer contains the data to be sent.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _MAPM_Send_Message_Listing(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int MessageListingLength, Byte_t *MessageListing, Boolean_t Final);

   /* The following function generates a MAP Message Listing Size       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  The Size parameter is the size of*/
   /* the message listing to return.  This function returns zero if     */
   /* successful and a negative return error code if there was an error.*/
int _MAPM_Send_Message_Listing_Size(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, Word_t MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime);

   /* The following function generates a MAP Get Message Response       */
   /* Response to the specified remote MAP Client.  The                 */
   /* RemoteDeviceAddress is the address of the remote client.  The     */
   /* InstanceID parameter specifies which server instance on the local */
   /* device to use.  The ResponseCode parameter is the OBEX Response   */
   /* Code to send with the response.  FractionalType indicates what    */
   /* sort of framented response this is.  The DataLength parameter     */
   /* specifies the length of the data.  The Buffer contains the data to*/
   /* be sent.  This function returns zero if successful and a negative */
   /* return error code if there was an error.                          */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.  Note that if the Get    */
   /*          Message Indication specified a non-fragmented            */
   /*          FractionalType then you must respond with the correct    */
   /*          non-fragmented FractionalType (i.e. ftMore or ftLast).   */
int _MAPM_Send_Message(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, MAP_Fractional_Type_t FractionalType, unsigned int MessageLength, Byte_t *Message, Boolean_t Final);

   /* The following function generates a MAP Set Message Status Response*/
   /* to the specified remote MAP Client.  The RemoteDeviceAddress is   */
   /* the address of the remote client.  The InstanceID parameter       */
   /* specifies which server instance on the local device to use.  The  */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  This function returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
int _MAPM_Set_Message_Status_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

   /* The following function generates a MAP Push Message Response to   */
   /* the specified remote MAP Client.  The RemoteDeviceAddress is the  */
   /* address of the remote client.  The InstanceID parameter specifies */
   /* which server instance on the local device to use.  The            */
   /* ResponseCode parameter is the OBEX Response Code to send with the */
   /* response.  The message handle is the handle for the client to     */
   /* refer to the message.  This function returns zero if successful   */
   /* and a negative return error code if there was an error.           */
int _MAPM_Push_Message_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode, char *MessageHandle);

   /* The following function generates a MAP Update Inbox Response to   */
   /* the specified remote MAP Client. The RemoteDeviceAddress is the   */
   /* address of the remote client. The InstanceID parameter specifies  */
   /* which server instance on the local device to use. The ResponseCode*/
   /* parameter is the OBEX Response Code to send with the response.    */
   /* This function returns zero if successful and a negative return    */
   /* error code if there was an error.                                 */
int _MAPM_Update_Inbox_Confirmation(BD_ADDR_t RemoteDeviceAddress, unsigned int InstanceID, unsigned int ResponseStatusCode);

#endif
