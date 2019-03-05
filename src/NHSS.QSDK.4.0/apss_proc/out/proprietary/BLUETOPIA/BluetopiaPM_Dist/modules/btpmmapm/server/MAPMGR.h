/*****< mapmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/* MAPMGR - Message Access Profile Manager Implementation for Stonestreet One */
/*          Bluetooth Protocol Stack Platform Manager.                        */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*                                                                            */
/******************************************************************************/
#ifndef __MAPMGRH__
#define __MAPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTMAPM.h"           /* MAP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Message Access Manager implementation.  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Message Access Manager implementation.                    */
int _MAPM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Message Access Manager implementation.  After this function is    */
   /* called the Message Access Manager implementation will no longer   */
   /* operate until it is initialized again via a call to the           */
   /* _MAPM_Initialize() function.                                      */
void _MAPM_Cleanup(void);

   /* The following function is responsible for informing the Message   */
   /* Access Manager implementation of that Bluetooth stack ID of the   */
   /* currently opened Bluetooth stack.  When this parameter is set to  */
   /* non-zero, this function will actually initialize the Message      */
   /* Access Manager with the specified Bluetooth stack ID.  When this  */
   /* parameter is set to zero, this function will actually clean up all*/
   /* resources associated with the prior initialized Bluetooth Stack.  */
void _MAPM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is responsible for opening a local MAP     */
   /* Server.  The parameter to this function is the Port on which to   */
   /* open this server, and *MUST* be between MAP_PORT_NUMBER_MINIMUM   */
   /* and MAP_PORT_NUMBER_MAXIMUM.  This function returns a positive,   */
   /* non zero value if successful or a negative return error code if an*/
   /* error occurs.  A successful return code will be a MAP Profile ID  */
   /* that can be used to reference the Opened MAP Profile Server Port  */
   /* in ALL other MAP Server functions in this module.  Once an MAP    */
   /* Profile Server is opened, it can only be Un-Registered via a call */
   /* to the _MAP_Close_Server() function (passing the return value from*/
   /* this function).                                                   */
int _MAP_Open_Message_Access_Server(unsigned int MessageAccessServiceServerPort);

   /* The following function is responsible for opening a local MAP     */
   /* NotificationServer.  The first parameter to this function is the  */
   /* port to use to open this server, and *MUST* be between            */
   /* MAP_PORT_NUMBER_MINIMUM and MAP_PORT_NUMBER_MAXIMUM.  The second  */
   /* parameter is the MAPID for the Message Access Client that the     */
   /* server is being associated with.  This function returns a         */
   /* positive, non zero value if successful or a negative return error */
   /* code if an error occurs.  A successful return code will be a MAP  */
   /* Profile ID that can be used to reference the Opened MAP Profile   */
   /* Server Port in ALL other MAP Server functions in this module.     */
   /* Once an MAP Profile Server is opened, it can only be Un-Registered*/
   /* via a call to the _MAP_Close_Server() function (passing the return*/
   /* value from this function).                                        */
int _MAP_Open_Message_Notification_Server(unsigned int MessageNotificationServiceServerPort, unsigned int MAS_MAPID);

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Server.  The first       */
   /* parameter to this function is the MAP ID of the Server for which a*/
   /* connection request was received.  The final parameter to this     */
   /* function specifies whether to accept the pending connection       */
   /* request (or to reject the request).  This function returns zero if*/
   /* successful, or a negative return error code if an error occurred. */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etMAP_Open_Service_Port_Indication event has occurred. */
int _MAP_Open_Request_Response(unsigned int MAPID, Boolean_t AcceptConnection);

   /* The following function adds a MAP Server (MSE) Service Record to  */
   /* the SDP Database.  The first parameter is the MAP ID that was     */
   /* returned by a previous call to _MAP_Open_Message_Access_Server(). */
   /* The second parameter is a pointer to ASCII, NULL terminated string*/
   /* containing the Service Name to include within the SDP Record.  The*/
   /* third parameter is a user defined MAS Instance value.  The fourth */
   /* parameter is the Supported Message Type Bitmask value.  This      */
   /* function returns a positive, non-zero value on success or a       */
   /* negative return error code if there was an error.  If this        */
   /* function returns success then the return value is the SDP Service */
   /* Record Handle of the Record that was added.                       */
   /* * NOTE * This function should only be called with the MAP ID that */
   /*          was returned from the _MAP_Open_Message_Access_Server()  */
   /*          function.  This function should NEVER be used with MAP ID*/
   /*          returned from the                                        */
   /*          _MAP_Open_Remote_Message_Access_Server_Port() function.  */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the _MAP_Un_Register_SDP_Record()  */
   /*          function.                                                */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
long _MAP_Register_Message_Access_Server_SDP_Record(unsigned int MAPID, char *ServiceName, Byte_t MASInstance, Byte_t SupportedMessageTypes);

   /* The following function adds a MAP Server (MSE) Service Record to  */
   /* the SDP Database.  The first parameter to this function is the MAP*/
   /* ID that was returned by a previous call to                        */
   /* _MAP_Open_Message_Access_Server().  The second parameter is a     */
   /* pointer to ASCII, NULL terminated string containing the Service   */
   /* Name to include within the SDP Record.  The third parameter is a  */
   /* user defined MAS Instance value.  The fourth parameter is the     */
   /* Supported Message Type Bitmask value.  This function returns a    */
   /* positive, non-zero value on success or a negative return error    */
   /* code if there was an error.  If this function returns success then*/
   /* the return value is the SDP Service Record Handle of the Record   */
   /* that was added.                                                   */
   /* * NOTE * This function should only be called with the MAP ID that */
   /*          was returned from the _MAP_Open_Message_Access_Server()  */
   /*          function.  This function should NEVER be used with MAP ID*/
   /*          returned from the                                        */
   /*          _MAP_Open_Remote_Message_Notification_Server_Port()      */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the MAP_Un_Register_SDP_Record()   */
   /*          function.                                                */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
long _MAP_Register_Message_Notification_Server_SDP_Record(unsigned int MAPID, char *ServiceName);

   /* The following function is responsible for deleting a previously   */
   /* registered MAP SDP Service Record.  This function accepts as its  */
   /* parameter the SDP Service Record Handle of the SDP Service Record */
   /* to delete from the SDP Database.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _MAP_Un_Register_SDP_Record(unsigned int MAPID, DWord_t ServiceRecordHandle);

   /* The following function is responsible for opening a connection to */
   /* a remote Message Access Server.  The first parameter to this      */
   /* function is the remote Bluetooth Device Address of the Bluetooth  */
   /* MAP Profile Server with which to connect.  The second parameter   */
   /* specifies the remote server port with which to connect.  The      */
   /* ServerPort parameter *MUST* be between MAP_PORT_NUMBER_MINIMUM and*/
   /* MAP_PORT_NUMBER_MAXIMUM.  This function returns a positive, non   */
   /* zero, value if successful or a negative return error code if an   */
   /* error occurs.  A successful return code will be a MAP ID that can */
   /* be used to reference the remote opened MAP Profile Server in ALL  */
   /* other MAP Profile Client functions in this module.  Once a remote */
   /* server is opened, it can only be closed via a call to the         */
   /* _MAP_Close_Connection() function (passing the return value from   */
   /* this function).                                                   */
int _MAP_Open_Remote_Message_Access_Server_Port(BD_ADDR_t BD_ADDR, unsigned int ServerPort);

   /* The following function is responsible for opening a connection to */
   /* a remote Message Notification Server.  The first parameter to this*/
   /* function is the MAP ID of the local Message Access Server making  */
   /* the connection.  The second parameter specifies the remote server */
   /* port with which to connect.  The ServerPort parameter *MUST* be   */
   /* between MAP_PORT_NUMBER_MINIMUM and MAP_PORT_NUMBER_MAXIMUM.  This*/
   /* function returns a positive, non zero, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a MAP ID that can be used to reference the    */
   /* remote opened MAP Profile Server in ALL other MAP Notification    */
   /* Client functions in this module.  Once a remote server is opened, */
   /* it can only be closed via a call to the _MAP_Close_Connection()   */
   /* function (passing the return value from this function).           */
int _MAP_Open_Remote_Message_Notification_Server_Port(unsigned int LocalMAPID, unsigned int ServerPort);

   /* The following function is responsible for closing a currently     */
   /* open/registered Message Access Profile server.  This function is  */
   /* capable of closing servers opened via a call to                   */
   /* _MAP_Open_Message_Access_Server() and                             */
   /* _MAP_Open_Message_Notification_Server().  The parameter to this   */
   /* function is the MAP ID of the Profile Server to be closed.  This  */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
int _MAP_Close_Server(unsigned int MAPID);

   /* The following function is responsible for closing a currently     */
   /* ongoing MAP Profile connection.  The parameter to this function is*/
   /* the MAP ID of the MAP Profile connection to be closed.  This      */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* * NOTE * If this function is called with a Server MAP ID (a value */
   /*          returned from a call to _MAP_Open_Server_Port()) any     */
   /*          clients currently connected to this server will be       */
   /*          terminated, but the server will remained open and        */
   /*          registered.  If this function is called using a Client   */
   /*          MAP ID (a value returned from a call to                  */
   /*          _MAP_Open_Remote_Server_Port()), the client connection   */
   /*          will be terminated/closed entirely.                      */
int _MAP_Close_Connection(unsigned int MAPID);

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Server.  The MAPID parameter specifies the MAP ID   */
   /* for the local MAP Client.  This function returns zero if          */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
int _MAP_Abort_Request(unsigned int MAPID);

   /* The following function is responsible for providing a mechanism to*/
   /* Enable or Disable Notification messages from the remote Message   */
   /* Access Server.  The first parameter to this function is the       */
   /* MAP ID of the Service Client making this call.  The second        */
   /* parameter specifies if the Notifications should be Enabled or     */
   /* Disabled.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.                               */
int _MAP_Set_Notification_Registration_Request(unsigned int MAPID, Boolean_t Enabled);

   /* The following function is responsible for sending a Notification  */
   /* Registration Response to the remote Client.  The first parameter  */
   /* to this function is the MAP ID of the Server making this call.    */
   /* The second parameter to this function is the Response Code to be  */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int _MAP_Set_Notification_Registration_Response(unsigned int MAPID, Byte_t ResponseCode);

   /* The following function is responsible for providing a mechanism   */
   /* for Message Notification Clients to dispatch an event to the      */
   /* remote Server.  The first parameter to this function is the MAP ID*/
   /* of the Service Client making this call.  The second parameter     */
   /* specifies the number of bytes that are present in the object      */
   /* segment (DataBuffer) The third parameter in a pointer to the      */
   /* segment of the event object data.  The final parameter to this    */
   /* function is a Boolean Flag indicating if this is to be the final  */
   /* segment of the Event Object.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The EventObject is a "x-bt/MAP-event-report" character   */
   /*          stream that is formatted as defined in the Message Access*/
   /*          Profile Specification.                                   */
int _MAP_Send_Event_Request(unsigned int MAPID, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent, Boolean_t Final);

   /* The following function is responsible for sending a Send Event    */
   /* Response to the remote Client.  The first parameter to this       */
   /* function is the MAP ID of the Server making this call.  The second*/
   /* parameter to this function is the Response Code to be associated  */
   /* with this response.  This function returns zero if successful, or */
   /* a negative return value if there was an error.                    */
int _MAP_Send_Event_Response(unsigned int MAPID, Byte_t ResponseCode);

   /* The following function generates a MAP Get Folder Listing Request */
   /* to the specified remote MAP Server.  The MAPID parameter specifies*/
   /* the MAP ID for the local MAP Client (returned from a successful   */
   /* call to the _MAP_Connect_Remote_Server_Port() function).  The     */
   /* MaxListCount parameter is an unsigned integer that specifies the  */
   /* maximum number of list entries the client can handle.  A          */
   /* MaxListCount of ZERO (0) indicates that this is a request for the */
   /* number accessible folders in the current folder.  The             */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this Folder Listing.  This function returns zero if     */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAP Profile Request    */
   /*          active at any one time.  Because of this, another MAP    */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the _MAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the MAP   */
   /*          Profile Event Callback that was registered when the MAP  */
   /*          Profile Port was opened).                                */
int _MAP_Get_Folder_Listing_Request(unsigned int MAPID, Word_t MaxListCount, Word_t ListStartOffset);

   /* The following function sends a MAP Get Folder Listing Response to */
   /* the specified remote MAP Client.  This is used for responding to a*/
   /* MAP Get Folder Listing Indication.  The MAPID parameter specifies */
   /* the MAP ID of the MAP Server responding to the request.  The      */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  The FolderCount parameter is a pointer to a variable   */
   /* that can optionally contain the number of accessible folder       */
   /* entries in the current folder.  This should ONLY be used if the   */
   /* received indication indicated a request for Folder Listing by     */
   /* indicating a MaxListCount = ZERO (0).  In all other instances this*/
   /* parameter must be set to NULL.  The DataLength parameter indicates*/
   /* the number of bytes that are contained in the object segment      */
   /* (DataBuffer).  The DataBuffer parameter is a pointer to a byte    */
   /* buffer containing a segment of the Folder Listing Object.  The    */
   /* AmountSent parameter is a pointer to variable which will be       */
   /* written with the actual amount of data that was able to be        */
   /* included in the packet.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The FolderListingObject is a "x-obex/folder-listing"     */
   /*          character stream that is formatted as defined in the     */
   /*          IrOBEX Specification.                                    */
   /* * NOTE * If FolderCount is not NULL, then the remaining parameters*/
   /*          are ignored.                                             */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
int _MAP_Get_Folder_Listing_Response(unsigned int MAPID, Byte_t ResponseCode, Word_t *FolderCount, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent);

   /* The following function generates a MAP Get Message Listing Request*/
   /* to the specified remote MAP Server.  The MAPID parameter specifies*/
   /* the MAP ID for the local MAP Client (returned from a successful   */
   /* call to the _MAP_Connect_Remote_Server_Port() function).  The     */
   /* FolderName specifies the Folder from which the Message Listing is */
   /* to be retrieved.  If the parameter is NULL, the listing is made   */
   /* from the current directory.  The MaxListCount parameter is an     */
   /* unsigned integer that specifies the maximum number of list entries*/
   /* the client can handle.  A MaxListCount of ZERO (0) indicates that */
   /* this is a request for the number of messages in the specified     */
   /* folder.  The ListStartOffset parameter specifies the index        */
   /* requested by the Client in this Listing.  The ListInfo structure  */
   /* is used to specify a number of filters and options that should be */
   /* applied to the request.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
int _MAP_Get_Message_Listing_Request(unsigned int MAPID, Word_t *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo);

   /* The following function sends a MAP Get Message Listing Response to*/
   /* the specified remote MAP Client.  This is used for responding to a*/
   /* MAP Get Message Listing Indication.  The MAPID parameter specifies*/
   /* the MAP ID of the MAP Server responding to the request.  The      */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  The MessageCount parameter contains the number of      */
   /* accessible message entries.  The NewMessage parameter indicates if*/
   /* new messages have arrived since last checked.  The CurrentTime    */
   /* parameters indicates the time at which the response is being sent.*/
   /* The DataLength parameter defines the number of bytes that are     */
   /* included in the object segment (DataBuffer).  The DataBuffer      */
   /* parameter is a pointer to a byte buffer containing a segment of   */
   /* the Message Listing Object.  The AmountSent parameter is a pointer*/
   /* to variable which will be written with the actual amount of data  */
   /* that was able to be included in the packet.  This function returns*/
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The MessageListingObject is a "x-bt/MAP-msg-listing"     */
   /*          character stream that is formatted as defined in the     */
   /*          Message Access Profile Specification.                    */
   /* * NOTE * If MessageCount is not NULL, then the remaining          */
   /*          parameters are ignored.                                  */
   /* * NOTE * CurrentTime is used by the receiver of this response to  */
   /*          correlate the timestamps in the listing with the current */
   /*          time of the server.                                      */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
int _MAP_Get_Message_Listing_Response(unsigned int MAPID, Byte_t ResponseCode, Word_t *MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent);

   /* The following function generates a MAP Get Message Request to the */
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  The MessageHandle parameter is a*/
   /* pointer to a 16 byte NULL terminated Unicode Text string that     */
   /* identifies the message.  The Attachment parameter is used to      */
   /* indicate if any existing attachments to the specified message are */
   /* to be included in the response.  The CharSet and Fractional Type  */
   /* parameters specify that format of the response message.  This     */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.                          */
int _MAP_Get_Message_Request(unsigned int MAPID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType);

   /* The following function sends a MAP Get Message Response to the    */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Get Message Indication.  The MAPID parameter specifies the MAP ID */
   /* of the MAP Server responding to the request.  The ResponseCode    */
   /* parameter is the OBEX response code to include in the response.   */
   /* The FractionalType parameter indicates the type of the object     */
   /* segment and can specify ftUnfragmented, ftMore or ftLast.  The    */
   /* DataLength parameter defines the number of bytes that are included*/
   /* in the object segment (DataBuffer).  The DataBuffer parameter is a*/
   /* pointer to a byte buffer containing the Message Listing Object    */
   /* segment.  The AmountSent parameter is a pointer to variable which */
   /* will be written with the actual amount of data that was able to be*/
   /* included in the packet.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The MessageObject is a "x-bt/message" character stream   */
   /*          that is formatted as defined in the Message Access       */
   /*          Profile Specification.                                   */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.  Note that if the Get    */
   /*          Message Indication specified a non-fragmented            */
   /*          FractionalType then you must respond with the correct    */
   /*          non-fragmented FractionalType (i.e. ftMore or ftLast).   */
int _MAP_Get_Message_Response(unsigned int MAPID, Byte_t ResponseCode, MAP_Fractional_Type_t FractionalType, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent);

   /* The following function generates a MAP Set Message Status Request */
   /* to the specified remote MAP Server.  The MAPID parameter specifies*/
   /* the MAP ID for the local MAP Client.  The MessageHandle parameter */
   /* is a pointer to a 16 byte NULL terminated Unicode Text string that*/
   /* identifies the message.  The StatusIndicator identifies the Status*/
   /* indicator to set.  The StatusValue indicates the new state of the */
   /* indicator.  This function returns zero if successful or a negative*/
   /* return error code if there was an error.                          */
int _MAP_Set_Message_Status_Request(unsigned int MAPID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue);

   /* The following function sends a MAP Set Message Status Response to */
   /* the specified remote MAP Client.  This is used for responding to a*/
   /* MAP Set Message Status Indication.  The MAPID parameter specifies */
   /* the MAP ID for the local MAP Client.  The ResponseCode parameter  */
   /* is the OBEX response code to include in the response.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _MAP_Set_Message_Status_Response(unsigned int MAPID, Byte_t ResponseCode);

   /* The following function generates a MAP Push Message Request to the*/
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  The FolderName parameter        */
   /* specifies the destination location for the message.  If this      */
   /* parameter is NULL, the current directory is used.  The Transparent*/
   /* parameter is used to indicate that no copy of the message should  */
   /* be placed in the Sent Folder.  Retry parameter is used to indicate*/
   /* if any attempts to retry the send if the previous attempt fails.  */
   /* The CharSet parameters specify that format of the message.  The   */
   /* DataLength parameter defines the number of bytes that are included*/
   /* in the object segment (DataBuffer).  The DataBuffer parameter is a*/
   /* pointer to a byte buffer containing the Message Listing Object    */
   /* segment.  The AmountSent parameter is a pointer to variable which */
   /* will be written with the actual amount of data that was able to be*/
   /* included in the packet.  The final parameter to this function is a*/
   /* Boolean Flag indicating if this is the last segment of the object.*/
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The MessageObject is a "x-bt/message" character stream   */
   /*          that is formatted as defined in the Message Access       */
   /*          Profile Specification.                                   */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
int _MAP_Push_Message_Request(unsigned int MAPID, Word_t *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent, Boolean_t Final);

   /* The following function sends a MAP Push Message Response to the   */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Push Message Indication.  The MAPID parameter specifies the MAP ID*/
   /* of the MAP Server responding to the request.  The ResponseCode    */
   /* parameter is the OBEX response code to include in the response.   */
   /* The MessageHandle parameter points to an ASCII string that        */
   /* contains 16 hexadecimal digits and represents the handle assigned */
   /* to the message pushed.                                            */
   /* * NOTE * The MessageHandle pointer must point to a valid string if*/
   /*          the Response Code indicates MAP_OBEX_RESPONSE_OK,        */
   /*          otherwise it may be NULL.                                */
int _MAP_Push_Message_Response(unsigned int MAPID, Byte_t ResponseCode, char *MessageHandle);

   /* The following function generates a MAP Update Inbox Request to the*/
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _MAP_Update_Inbox_Request(unsigned int MAPID);

   /* The following function sends a MAP Update Inbox Response to the   */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Set Message Status Indication.  The MAPID parameter specifies the */
   /* MAP ID for the local MAP Client.  The ResponseCode parameter is   */
   /* the OBEX response code to include in the response.  This function */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _MAP_Update_Inbox_Response(unsigned int MAPID, Byte_t ResponseCode);

   /* The following function generates a MAP Set Folder Request to the  */
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  The PathOption parameter        */
   /* contains an enumerated value that indicates the type of path      */
   /* change to request.  The FolderName parameter contains the folder  */
   /* name to include with this SetFolder request.  This value can be   */
   /* NULL if no name is required for the selected PathOption.  See the */
   /* MAP specification for more information.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAP Profile Request    */
   /*          active at any one time.  Because of this, another MAP    */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the MAP_Abort_Request()   */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the MAP   */
   /*          Profile Event Callback that was registered when the MAP  */
   /*          Profile Port was opened).                                */
int _MAP_Set_Folder_Request(unsigned int MAPID, MAP_Set_Folder_Option_t PathOption, Word_t *FolderName);

   /* The following function sends a MAP Set Folder Response to the     */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Set Folder Indication.  The MAPID parameter specifies the MAP ID  */
   /* for the local MAP Client.  The ResponseCode parameter is the OBEX */
   /* response code to include in the response.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _MAP_Set_Folder_Response(unsigned int MAPID, Byte_t ResponseCode);

#endif
