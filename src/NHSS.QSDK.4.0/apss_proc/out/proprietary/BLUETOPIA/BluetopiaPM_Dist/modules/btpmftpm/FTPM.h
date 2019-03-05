/*****< ftpm.h >***************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FTPM - FTP Manager Implementation for Stonestreet One Bluetooth Protocol  */
/*         Stack Platform Manager.                                            */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/03/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FTPMH__
#define __FTPMH__

#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */
#include "BTTypes.h"            /* Bluetooth Type Definitions/Constants.      */

#include "SS1BTPS.h"            /* Bluetopia Core Prototypes/Constants.       */

   /* The following declared type represents the Prototype Function for */
   /* an OTPM Event Callback.  This function will be called whenever a  */
   /* defined OTPM Action occurs.  This function passes to the caller   */
   /* the OTPM Event Data associated with the OTPM Event that occurred, */
   /* and the OTPM Callback Parameter that was specified when this      */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the OTPM Event Data ONLY in the context of this callback.  If the */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).                             */
typedef void (BTPSAPI *_FTPM_Event_Callback_t)(OTP_Event_Data_t *OTP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for initializing an FTPM Mgr*/
   /* Context Layerk.  This function returns zero if successful, or a   */
   /* non-zero value if there was an error.                             */
int _FTPM_Initialize(void);

   /* The following function is responsible for releasing any resources */
   /* that the FTPM Mgr Layer has allocated.  Upon completion of this   */
   /* function, ALL FTPM Mgr functions will fail until _FTPM_Initialize */
   /* is called again successfully.                                     */
void _FTPM_Cleanup(void);

   /* The following function is responsible for establishing an FTPM Mgr*/
   /* Port Server (will wait for a connection to occur on the port      */
   /* established by this function).  This function accepts as input the*/
   /* Port Number to establish, followed by the Port Flags.  The third  */
   /* parameter specifies the type of OBEX Server that is to be         */
   /* established (File Browser, IrSync, or Inbox).  The function also  */
   /* takes as a parameter the Maximum Packet Length that will be       */
   /* accepted for this server.  If the value supplied is outside the   */
   /* valid range, then the value used will be the valid value closest  */
   /* to the value supplied.  The last two parameters specify the FTPM  */
   /* Mgr Event Callback function and Callback Parameter, respectively, */
   /* that will be called with FTPM Mgr Events that occur on the        */
   /* specified FTPM Mgr Port.  This function returns a non-zero,       */
   /* positive, number on success or a negative return error code if an */
   /* error occurred.  A successful return code will be a Port ID that  */
   /* can be used to reference the Opened FTPM Mgr Port in ALL other    */
   /* functions in this module (except the _FTPM_Open_Remote_Port()     */
   /* function).  Once a Server Port is opened, it can only be          */
   /* Un-Registered via a call to the _FTPM_Close_Server_Port() function*/
   /* (passing the return value from this function).  The               */
   /* _FTPM_Close_Port() function can be used to Disconnect a Client    */
   /* from the Server Port (if one is connected, it will NOT Un-Register*/
   /* the Server Port however).                                         */
int _FTPM_Open_Server_Port(unsigned int ServerPort, unsigned long PortFlags, OTP_Target_t Target, Word_t MaxPacketLength, _FTPM_Event_Callback_t EventCallback, unsigned long CallbackParameter);

   /* The following function is responsible for Un-Registering an FTPM  */
   /* Mgr Port Server (which was Registered by a successful call to the */
   /* _FTPM_Open_Server_Port() function).  This function accepts as     */
   /* input the FTPM Mgr Server ID that is registered.  This function   */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
int _FTPM_Close_Server_Port(unsigned int OTP_ID);

   /* The following function is responsible for responding to requests  */
   /* to connect to an FTPM Mgr Port Server.  This function accepts as  */
   /* input the FTPM Mgr Port ID (which *MUST* have been obtained by    */
   /* calling the _FTPM_Open_Server_Port() function), and as the final  */
   /* parameter whether to accept the pending connection.  This function*/
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
int _FTPM_Open_Port_Request_Response(unsigned int OTP_ID, Boolean_t AcceptConnection);

   /* The following function is provided to allow a means to add a      */
   /* Generic OBEX Service Record to the SDP Database.  This function   */
   /* takes as input the FTPM Mgr Port ID (which *MUST* have been       */
   /* obtained by calling the _FTPM_Open_Server_Port() function.  The   */
   /* second parameter (required) specifies any additional SDP          */
   /* Information to add to the record.  The third parameter specifies  */
   /* the Service Name to associate with the SDP Record.  The final     */
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the FTPM Mgr    */
   /*          Port ID that was returned from the                       */
   /*          _FTPM_Open_Server_Port() function.  This function should */
   /*          NEVER be used with the FTPM Mgr Port ID returned from the*/
   /*          _FTPM_Open_Remote_Port() function.                       */
   /* * NOTE * There must be UUID Information specified in the          */
   /*          SDPServiceRecord Parameter, however protocol information */
   /*          is completely optional.  Any Protocol Information that is*/
   /*          specified (if any) will be added in the Protocol         */
   /*          Attribute AFTER the default OBEX Protocol List (L2CAP,   */
   /*          RFCOMM, and OBEX).                                       */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
int _FTPM_Register_SDP_Record(unsigned int OTP_ID, OTP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);

   /* The following function is responsible for Opening a port to a     */
   /* remote Server on the specified Server Port.  This function accepts*/
   /* the BD_ADDR of the remote server.  The second parameter specifies */
   /* the port on which the server is attached.  The third parameter    */
   /* specifies the optional flags to use when opening the port.  The   */
   /* next parameter specifies the Max Packet Length that this client is*/
   /* capable of receiving.  The final two parameters specify the Event */
   /* Callback function, and callback parameter, respectively, of the   */
   /* Event Callback that is to process the FTPM Mgr Events.  This      */
   /* function returns a non-zero, positive, value if successful, or a  */
   /* negative return error code if this function is unsuccessful.  If  */
   /* this function is successful, the return value will represent the  */
   /* FTPM Mgr ID that can be passed to all other functions that require*/
   /* it.  Once a remote server is opened, it can only be closed via a  */
   /* call to the _FTPM_Close_Port() function (passing the return value */
   /* from this function).                                              */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int _FTPM_Open_Remote_Port(BD_ADDR_t BD_ADDR, unsigned int ServerPort, unsigned long OpenFlags, Word_t MaxPacketLength, _FTPM_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ConnectionStatus);

   /* The following function is used to terminate a possible connection */
   /* to a remote server or client.  If this is called by a Server, the */
   /* connection to the client will be terminated, but the Server will  */
   /* remain registered.  This function accepts as input the Port ID    */
   /* that was returned in the _FTPM_Open_Server_Port() or the          */
   /* _FTPM_Open_Remote_Port().  This function returns zero if          */
   /* successful, or a negative return value if there was an error.     */
   /* This function does NOT Un-Register a Server Port from the system, */
   /* it ONLY disconnects any connection that is currently active on the*/
   /* Server Port.  The _FTPM_Close_Server_Port() function can be used  */
   /* to Un-Register the Server.                                        */
int _FTPM_Close_Port(unsigned int OTP_ID);

   /* The following function is used to Send an OBEX Connect Request to */
   /* the Remote Server.  The remote server is referenced by the FTPM   */
   /* Mgr ID that was returned from an _FTPM_Open_Remote_Port.  This    */
   /* function accepts as input the OTP_ID parameter which references   */
   /* the connection on which the connect is to be sent, obtained from  */
   /* the Open Port function.  The Target parameter identifies the      */
   /* service on the remote server to which the connection is targeted. */
   /* The DigestChallenge and DigestResponse parameters are used to pass*/
   /* Authentication Request and Response information between Server and*/
   /* Clients.  These parameters should be set to NULL if authentication*/
   /* is not in use.  This function returns zero if successful, or a    */
   /* negative return value if there was an error.                      */
int _FTPM_Client_Connect(unsigned int OTP_ID, OTP_Target_t Target, OTP_Digest_Challenge_t *DigestChallenge, OTP_Digest_Response_t *DigestResponse);

   /* The following function is used to Disconnect an OBEX connection.  */
   /* This function will disconnect from an OBEX service on the remote  */
   /* OBEX server without releasing the connection to the Server.  This */
   /* function accepts as input the OTP_ID parameter which references   */
   /* the connection that is to be disconnected.  This function returns */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
int _FTPM_Client_Disconnect(unsigned int OTP_ID);

   /* The following function is used to request a directory listing from*/
   /* a remote OBEX File Browsing Server.  The function takes as its    */
   /* first parameter The OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The Name parameter*/
   /* is a pointer to a ASCIIZ string that identifies the name of the   */
   /* directory that is to be retreived.  When specifying the Name, No  */
   /* path information is allowed.  When retreiving a directory listing,*/
   /* the SETPATH function should be used to set the current directory. */
   /* This function is then called with the Name parameter set to NULL  */
   /* to pull the current directory.  If the Name parameter is not NULL,*/
   /* then Name must point to a ASCIIZ string of the name of a          */
   /* sub-directory that exists off the current directory.  It must also*/
   /* be noted that when the Name parameter is used, a sub-directory    */
   /* listing will be returned for the directory specified, however, the*/
   /* current directory will remain the same and will not be changed to */
   /* the sub-directory specified.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The Name parameter should be formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
int _FTPM_Client_Get_Directory(unsigned int OTP_ID, char *Name);

   /* The following function is used to Pull and Object from a Remote   */
   /* OBEX Server.  The function takes as its first parameter the OTP_ID*/
   /* parameter which references the OBEX Connection on which the       */
   /* request is to be made.  The Type parameter is a pointer to a NULL */
   /* terminated string that describes the type of object to be         */
   /* retreived.  The Name parameter is a pointer to a NULL terminated  */
   /* string that specifies the Name of the Object that is to be        */
   /* retreived.  UserInfo is a user defined parameter.  This UserInfo  */
   /* parameter will be returned in the associated Get Response         */
   /* Callback.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.  It should be noted that when */
   /* connected to an OBEX File Browser Service, the Type parameter is  */
   /* optional.  When connected to the OBEX Inbox, the Name parameter is*/
   /* optional.                                                         */
   /* * NOTE * The Type and Name parameters should be formatted as NULL */
   /*          terminated ASCII strings with UTF-8 encoding.            */
int _FTPM_Client_Get_Object(unsigned int OTP_ID, char *Type, char *Name, unsigned long UserInfo);

   /* The following function is used to send a request to an OBEX Server*/
   /* to save or create an object on the Server.  The function takes as */
   /* its first parameter the OTP_ID parameter which references the OBEX*/
   /* Connection on which the request is to be made.  The CreateOnly    */
   /* parameter specifies whether or not this request is being made to  */
   /* put an object (CreateOnly equals FALSE), or simply create an      */
   /* object of zero length (CreateOnly equals TRUE).  The length field */
   /* specifies the total size (in bytes of the Object).  The Type      */
   /* parameter is a pointer to a NULL terminated string that identifies*/
   /* the Type of object the request is for.  The Name parameter is a   */
   /* NULL terminated string that identifies the name of the object for */
   /* which the request is for.  UserInfo is a user defined parameter.  */
   /* This UserInfo parameter will be returned in the associated Put    */
   /* Response callback.  This function returns zero if successful, or a*/
   /* negative return value if there was an error.  It should be noted  */
   /* that when connected to an OBEX File Browser Service, the Type     */
   /* parameter is optional.  When connected to the OBEX Inbox, the Name*/
   /* parameter is optional.                                            */
   /* * NOTE * The Type and Name parameters should be formatted as NULL */
   /*          terminated ASCII strings with UTF-8 encoding.            */
int _FTPM_Client_Put_Object_Request(unsigned int OTP_ID, Boolean_t CreateOnly, unsigned int Length, char *Type, char *Name, unsigned long UserInfo);

   /* This function is used to send an object to a remote OBEX Server.  */
   /* This function must be used after an acceptable response is        */
   /* received from the _FTPM_Client_Put_Object_Request and is used to  */
   /* transfer the object itself.  The function takes as its first      */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  The DataLength    */
   /* parameter specifies the number of bytes that are to be transferred*/
   /* in this packet.  The Data parameter is a pointer to DataLength    */
   /* number of bytes that are to be transferred.  The Final parameter  */
   /* is a Boolean_t that denotes if the data that is supplied via the  */
   /* Data parameter is the last block of object data to be transferred.*/
   /* UserInfo is a user defined parameter.  This UserInfo parameter    */
   /* will be returned in the associated Put Response callback.  This   */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
int _FTPM_Client_Put_Object(unsigned int OTP_ID, unsigned int DataLength, Byte_t *Data, Boolean_t Final, unsigned long UserInfo);

   /* The following function is used to create, delete or set the       */
   /* current directory, of remote OBEX server supplying File Browsing  */
   /* Services.  The function takes as its first parameter the OTP_ID   */
   /* parameter which references the OBEX Connection on which the       */
   /* request is to be made.  The Name parameter is a pointer to a NULL */
   /* terminated string that specifies the name of a sub-directory with */
   /* reference to the current directory to which the path is to be set.*/
   /* The Backup parameter is used to request that the path be set to   */
   /* the next higher level.  The Create parameter is used to specify   */
   /* that the directory is to be created if it does not already exist. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.  Note that the Backup flag has the   */
   /* highest priority and The Name parameter will be ignored when      */
   /* Backup is set TRUE.  Also, when the Create parameter is TRUE the  */
   /* Name parameter must also be specified.                            */
   /* * NOTE * The Name parameter should be formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
int _FTPM_Client_Set_Path(unsigned int OTP_ID, char *Name, Boolean_t Backup, Boolean_t Create);

   /* The following function is used to send a request to an OBEX Server*/
   /* to delete an object on the Server.  The function takes as its     */
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The Name parameter*/
   /* is a NULL terminated string that identifies the name of the object*/
   /* for which the request is for.  This function returns zero if      */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The Name parameter should be formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
int _FTPM_Client_Delete_Object_Request(unsigned int OTP_ID, char *Name);

   /* The following function is used to Abort a request to the remote   */
   /* server that is outstanding.  The function takes as its first      */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
int _FTPM_Client_Abort_Request(unsigned int OTP_ID);

   /* The following function is used to send a response to a remote     */
   /* client due to a request for connection.  The function takes as its*/
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The parameter     */
   /* Accept is used to indicate if the connection request is being     */
   /* accepted or rejected.  When authentication is required for the    */
   /* connection, the structure DigestChallenge and DigestResponse is   */
   /* used to pass the Authentication information.  If authentication is*/
   /* not being used, these parameters should be set to NULL.  The      */
   /* DigestChallenge parameter is used to initiate authentication of   */
   /* the remote Client.  The DigestResponse is used to respond to a    */
   /* Challenge request from the remote client.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
int _FTPM_Connect_Response(unsigned int OTP_ID, Boolean_t Accept, OTP_Digest_Challenge_t *DigestChallenge, OTP_Digest_Response_t *DigestResponse);

   /* The following function is used to send a response to a remote     */
   /* client due to a request for a Directory listing.  The function    */
   /* takes as its first parameter the OTP_ID parameter which references*/
   /* the OBEX Connection on which the request is to be made.  The      */
   /* parameter DirEntry is a pointer to an array of directory entry    */
   /* structures.  Each entry in the array contains information about a */
   /* file or directory entry that is to be sent in response to the     */
   /* request.  It is important to note that the stack receives the     */
   /* directory information as an array of structures, and will convert */
   /* this information into XML format prior to sending to information  */
   /* to the remote client.  The process of converting the data to XML  */
   /* and sending all of the information to the remote client may       */
   /* require multiple requests and responses from the client and       */
   /* server.  The lower layer stack will handle all of these additional*/
   /* transactions without any further interaction from the application.*/
   /* Since the directory transfer process may take some time to        */
   /* complete, the data pointed to by the parameter DirInfo must be    */
   /* preserved until the transfer process is complete.  When the       */
   /* DirInfo information is no longer needed by the lower stack, a     */
   /* Callback will be generated with the                               */
   /* etOTP_Free_Directory_Information event to inform the application  */
   /* that the directory transfer process is complete and the data can  */
   /* be freed.  The parameter ResponseCode is used to notify the remote*/
   /* client of its ability to satisfy the request.  If the ResponseCode*/
   /* value is non-Zero, then the information pointed to by the DirInfo */
   /* parameter is considered invalid and the ResponseCode value        */
   /* represents the OBEX result code that identifies the reason why the*/
   /* request was not processed.  This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
int _FTPM_Get_Directory_Request_Response(unsigned int OTP_ID, OTP_DirectoryInfo_t *DirInfo, Byte_t ResponseCode);

   /* The following function is used to respond to a request from a     */
   /* remote Client to Change to a new directory, Create, or Delete a   */
   /* directory.  The function takes as its first parameter the OTP_ID  */
   /* parameter which references the OBEX Connection on which the       */
   /* request is to be made.  The parameter ResponseCode is used to     */
   /* notify the remote client of its ability to satisfy the request.   */
   /* If the ResponseCode value is non-Zero, then the request could not */
   /* be satisfied and the ResponseCode parameter contains the OBEX     */
   /* result code that identifies the reason why the request was not    */
   /* processed.  This function returns zero if successful, or a        */
   /* negative return value if there was an error.                      */
int _FTPM_Set_Path_Response(unsigned int OTP_ID, Byte_t ResponseCode);

   /* The following function is used to respond to a request from a     */
   /* remote Client to Abort an operation.  The function takes as its   */
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  It is not possible*/
   /* to refuse an abort request, so no further parameters are required.*/
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
int _FTPM_Abort_Response(unsigned int OTP_ID);

   /* The following function is used to respond to a request from a     */
   /* remote Client to Get an Object.  The function takes as its first  */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  When the Get      */
   /* Request was received from the Client, the Application was provided*/
   /* a pointer to a Buffer where the data is to be loaded.  The buffer */
   /* provided to the application is referenced in the OTP_Info_t       */
   /* structure.  The parameter BytesToSend indicates the number of     */
   /* Bytes of data that the application has loaded in this buffer.  The*/
   /* parameter ResponseCode is used to notify the remote client of its */
   /* ability to satisfy the request.  If the ResponseCode value is     */
   /* non-zero, then the request could not be satisfied and the         */
   /* ResponseCode parameter contains the OBEX result code that         */
   /* identifies the reason why the request was not processed.  The     */
   /* parameter UserInfo is a user defined parameter.  The value of this*/
   /* parameter will be passed back to the application on the next Get  */
   /* Request event.  This function returns zero if successful, or a    */
   /* negative return value if there was an error.                      */
int _FTPM_Get_Object_Response(unsigned int OTP_ID, unsigned int BytesToSend, unsigned int ResponseCode, unsigned long UserInfo);

   /* The following function is used to respond to a request from a     */
   /* remote Client to Delete an Object.  The function takes as its     */
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The parameter     */
   /* ResponseCode is used to notify the remote client of its ability to*/
   /* satisfy the request.  If the ResponseCode value is non-zero, then */
   /* the request could not be satisfied and the ResponseCode parameter */
   /* contains the OBEX result code that identifies the reason why the  */
   /* request was not processed.  This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
int _FTPM_Delete_Object_Response(unsigned int OTP_ID, Byte_t ResponseCode);

   /* The following function is used to respond to a request from a     */
   /* remote Client to Put an Object.  The function takes as its first  */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  The parameter     */
   /* ResponseCode is used to notify the remote client of its ability to*/
   /* satisfy the request.  If the ResponseCode value is non-zero, then */
   /* the request could not be satisfied and the ResponseCode parameter */
   /* contains the OBEX result code that identifies the reason why the  */
   /* request was not processed.  This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
int _FTPM_Put_Object_Response(unsigned int OTP_ID, Byte_t ResponseCode);

#endif
