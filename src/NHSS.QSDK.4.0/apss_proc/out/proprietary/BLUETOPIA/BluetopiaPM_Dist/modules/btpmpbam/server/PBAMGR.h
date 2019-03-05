/*****< pbamgr.h >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PBAMGR - Phone Book Access Manager Implementation for Stonestreet One     */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __PBAMGRH__
#define __PBAMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPBAM.h"           /* PBA Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Phone Book Access Manager implementation. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Phone Book Access Manager implementation.                 */
int _PBAM_Initialize(PBAM_Initialization_Info_t *PBAMInitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* Phone Book Access Manager implementation. After this function     */
   /* is called the Phone Book Access Manager implementation will no    */
   /* longer operate until it is initialized again via a call to the    */
   /* _PBAM_Initialize() function.                                      */
void _PBAM_Cleanup(void);

   /* The following function is responsible for informing the Phone Book*/
   /* Access Manager implementation of that Bluetooth stack ID of the   */
   /* currently opened Bluetooth stack. When this parameter is set to   */
   /* non-zero, this function will actually initialize the Phone Book   */
   /* Access Manager with the specified Bluetooth stack ID. When this   */
   /* parameter is set to zero, this function will actually clean up all*/
   /* resources associated with the prior initialized Bluetooth Stack.  */
void _PBAM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Phone Book Access device.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function accepts the connection */
   /* information for the remote device (address and server port).      */
int _PBAM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort);

   /* The following function exists to close an active Phone Book Access*/
   /* connection that was previously opened by a Successful call to     */
   /* _PBAM_Connect_Remote_Device() function.  This function accepts as */
   /* input the type of the local connection which should close its     */
   /* active connection.  This function returns zero if successful, or a*/
   /* negative return value if there was an error.                      */
int _PBAM_Disconnect_Device(unsigned int PBAPID);

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding PBAP Profile Client Request.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Stack for which the PBAP  */
   /* Profile Client is valid.  The second parameter to this function   */
   /* specifies the PBAP ID (returned from a successful call to the     */
   /* _PBAM_Connect_Remote_Device() function).  This function returns   */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a PBAP Profile Client Request that is to be aborted */
   /*          may have completed before the server was able to Abort   */
   /*          the request.  In either case, the caller will be notified*/
   /*          via PBAP Profile Callback of the status of the previous  */
   /*          Request.                                                 */
int _PBAM_Abort_Request(unsigned int PBAPID);

   /* The following function generates a PBAP Pull Phone Book request   */
   /* to the specified remote PBAP Server. The PBAPID parameter         */
   /* specifies the PBAP ID for the local PBAP Client (returned from a  */
   /* successful call to the _PBAM_ConnectRemoteDevice() function). The */
   /* PhoneBookNamePath parameter contains the name/path of the phone   */
   /* book being requested by this pull phone book operation. This value*/
   /* can be NULL if a phone book size request is being performed. The  */
   /* FilterLow parameter contains the lower 32 bits of the 64-bit      */
   /* filter attribute. The FilterHigh parameter contains the higher 32 */
   /* bits of the 64-bit filter attribute. The Format parameter is an   */
   /* enumeration which specifies the vCard format requested in this    */
   /* pull phone book request. If pfDefault is specified then the format*/
   /* will not be included in the request (note that the server will    */
   /* default to pfvCard21 in this case). The MaxListCount parameter    */
   /* is an unsigned integer that specifies the maximum number of       */
   /* entries the client can handle. A value of 65535 means that the    */
   /* number of entries is not restricted. A MaxListCount of ZERO (0)   */
   /* indicates that this is a request for the number of used indexes in*/
   /* the Phonebook specified by the PhoneBookNamePath parameter. The   */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this PullPhonebookRequest. This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP profile server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP profile server successfully */
   /*          executed the request.                                    */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the _PBAM_AbortRequest()  */
   /*          function) or the current request is completed (this is   */
   /*          signified by receiving a confirmation event in the PBAP  */
   /*          profile event callback that was registered when the PBAM */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_Phone_Book_Request(unsigned int PBAPID, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t Format, Word_t MaxListCount, Word_t ListStartOffset);

   /* The following function generates a PBAP Set Phonebook Request to  */
   /* the specified remote PBAP Server.  The BluetoothStackID parameter */
   /* the ID of the Bluetooth Stack that is associated with this PBAP   */
   /* Client.  The PBAPID parameter specifies the PBAP ID for the local */
   /* PBAP Client (returned from a successful call to the               */
   /* _PBAM_Connect_Remote_Device() function).  The PathOption parameter*/
   /* contains an enumerated value that indicates the type of path      */
   /* change to request.  The ObjectName parameter contains the folder  */
   /* name to include with this Set Phonebook request.  This value can  */
   /* be NULL if no name is required for the selected PathOption.  See  */
   /* the PBAP specification for more information.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Set_Phone_Book_Request(unsigned int PBAPID, PBAM_Set_Path_Option_t PathOption, char *ObjectName);

   /* The following function generates a PBAP Pull vCard Listing        */
   /* Request to the specified remote PBAP Server. The PBAPID parameter */
   /* specifies the PBAP ID for the local PBAP Client (returned from a  */
   /* successful call to the _PBAM_Connect_Remote_Device() function).   */
   /* The ObjectName parameter contains the folder of the Phonebook     */
   /* being requested by this Pull vCard Listing operation. This value  */
   /* can be NULL if a PhonebookSize request is being performed. The    */
   /* ListOrder parameter is an enumerated type that determines the     */
   /* optionally requested order of the listing. Using the 'loDefault'  */
   /* value for this parameter will prevent this field from being added */
   /* to the request (note that the server will default to loIndexed    */
   /* in this case). The SearchAttribute is an enumerated type that     */
   /* determines the optionally requested attribute used to filter this */
   /* request. Using the 'saDefault' value for this parameter will      */
   /* prevent this field from being added to the request (note that the */
   /* server will default to saIndexed in this case). The SearchValue   */
   /* parameter contains an optional ASCII, Null-terminated character   */
   /* string that contains the string requested for search/filter. If   */
   /* this parameter is NULL, this field will be excluded from the      */
   /* request. The MaxListCount parameter is an unsigned integer that   */
   /* specifies the maximum number of list entries the client can       */
   /* handle. A value of 65535 means that the number of entries is not  */
   /* restricted. A MaxListCount of ZERO (0) indicates that this is a   */
   /* request for the number of used indexes in the Phonebook specified */
   /* by the ObjectName parameter. The ListStartOffset parameter        */
   /* specifies the index requested by the Client in this Pull vCard    */
   /* Listing. This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the PBAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_vCard_Listing_Request(unsigned int PBAPID, char *ObjectName, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset);

   /* The following function generates a PBAP Pull vCard Entry Request  */
   /* to the specified remote PBAP Server. The PBAPID parameter         */
   /* specifies the PBAP ID for the local PBAP Client (returned from a  */
   /* successful call to the _PBAM_Connect_Remote_Device() function).   */
   /* The ObjectName parameter contains the name of the Phonebook       */
   /* entry being requested by this Pull vCard Entry operation. The     */
   /* FilterLow parameter contains the lower 32 bits of the 64-bit      */
   /* filter attribute. The FilterHigh parameter contains the higher 32 */
   /* bits of the 64-bit filter attribute. The Format parameter is an   */
   /* enumeration which specifies the vCard format requested in this    */
   /* Pull vCard Entry request. If pfDefault is specified then the      */
   /* format will not be included in the request (note that in this case*/
   /* the server will default to pfvCard21 in this case). This function */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the PBAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_vCard_Entry_Request(unsigned int PBAPID, char *ObjectName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t Format);

   /* Phonebook Access - PSE function prototypes.                       */

   /* The following function is responsible for opening a local PBAP    */
   /* Server.  The ServerPort parameter is the Port on which to open    */
   /* this server, and *MUST* be between PBAP_PORT_NUMBER_MINIMUM and   */
   /* PBAP_PORT_NUMBER_MAXIMUM.  The SupportedRepositories parameter    */
   /* is a bitmask which determines which repositories are supported    */
   /* by this server instance.  This function returns a positive, non   */
   /* zero value if successful or a negative return error code if an    */
   /* error occurs.  A successful return code will be a PBAP Profile ID */
   /* that can be used to reference the Opened PBAP Profile Server Port */
   /* in ALL other PBAP Server functions in this module.  Once an PBAP  */
   /* Profile Server is opened, it can only be Un-Registered via a call */
   /* to the PBAP_Close_Server() function (passing the return value from*/
   /* this function).                                                   */
int _PBAM_Open_Server(unsigned int ServerPort, Byte_t SupportedRepositories);

   /* The following function is responsible for closing a PBAP          */
   /* Profile Server (which was opened by a successful call to the      */
   /* PBAM_Open_Server() function).  The first parameter is the         */
   /* Bluetooth Stack ID of the previously opened server port.  The     */
   /* second parameter is the PBAP ID returned from the previous call to*/
   /* PBAM_Open_Server().  This function returns zero if successful, or */
   /* a negative return error code if an error occurred.  Note that this*/
   /* function does NOT delete any SDP Service Record Handles.          */
int _PBAM_Close_Server(unsigned int PBAPID);

   /* The following function adds a PBAP Server (PSE) Service Record    */
   /* to the SDP Database.  The PBAPID parameter is the PBAP ID that    */
   /* was returned by a previous call to PBAP_Open_Server_Port().  The  */
   /* ServiceName parameter is a pointer to ASCII, NULL terminated      */
   /* string containing the Service Name to include within the SDP      */
   /* Record.  The ServiceRecordHandle parameter is a pointer to a      */
   /* DWord_t which receives the SDP Service Record Handle if this      */
   /* function successfully creates an SDP Service Record.  If this     */
   /* function returns zero, then the SDPServiceRecordHandle entry will */
   /* contain the Service Record Handle of the added SDP Service Record.*/
   /* If this function fails, a negative return error code will be      */
   /* returned and the SDPServiceRecordHandle value will be undefined.  */
int _PBAM_Register_Service_Record(unsigned int PBAPID, char *ServiceName, DWord_t *ServiceRecordHandle);

   /* This function removes a PBAP Server (PSE) Service Record from     */
   /* the SDP database. The PBAPID parameter is the PBAP ID that was    */
   /* returned by a previous call to PBAM_Register_Server(). The        */
   /* ServiceRecordHandle is the handle of the PBAP service record to   */
   /* remove. This function returns zero if successful and a negative   */
   /* return error code if there is an error.                           */
int _PBAM_Un_Register_Service_Record(unsigned int PBAPID, DWord_t ServiceRecordHandle);

   /* The following function is provided to allow a means to respond    */
   /* to a request to connect to the local PBAP Profile Server.  The    */
   /* first parameter is the PBAP ID that was returned from a previous  */
   /* PBAP_Open_Server_Port() function for this server.  The final      */
   /* parameter to this function is a Boolean_t that indicates whether  */
   /* to accept the pending connection.  This function returns zero if  */
   /* successful, or a negative return value if there was an error.     */
int _PBAM_Open_Request_Response(unsigned int PBAPID, Boolean_t Accept);

   /* The following function sends a PBAP Pull Phonebook Response to the*/
   /* specified remote PBAP Client.  This is used for responding to a   */
   /* PBAP Pull Phonebook Indication.  The PBAPID parameter specifies   */
   /* the PBAP ID of the PBAP Server responding to the request.  The    */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  The PhonebookSize parameter is a pointer to a variable */
   /* that can optionally contain a Phonebook Size value to return in   */
   /* this request.  This should be done if the received indication     */
   /* indicated a request for PhonebookSize by indicating a MaxListCount*/
   /* = ZERO (0).  If this value is to be included in the response the  */
   /* Buffer parameter should be set to NULL and the BufferSize to ZERO.*/
   /* If this value is NOT to be used in the response, this parameter   */
   /* should be set to NULL.  The NewMissedCalls parameter is a pointer */
   /* to a variable that can optionally contain the number of new missed*/
   /* calls which have not been checked on this server.  This should    */
   /* only be included on requests for the 'mch' phonebook type.  If    */
   /* this value is to be included in the response the Buffer parameter */
   /* should be set to NULL and the BufferSize to ZERO.  If this value  */
   /* is NOT to be used in the response, this parameter should be set   */
   /* to NULL.  The BufferSize parameter is the size in bytes of the    */
   /* data included in the specified Buffer.  The Buffer parameter is   */
   /* a pointer to a byte buffer containing the Phonebook data to be    */
   /* included in this response packet.  The AmountWritten parameter is */
   /* a pointer to variable which will be written with the actual amount*/
   /* of data that was able to be included in the packet.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PBAM_Pull_Phonebook_Response(unsigned int PBAPID, Byte_t ResponseCode, Word_t *PhonebookSize, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);

   /* The following function sends a PBAP Set Phonebook Response to the */
   /* specified remote PBAP Client.  This is used for responding to a   */
   /* PBAP Set Phonebook Indication.  The PBAPID parameter specifies    */
   /* the PBAP ID of the PBAP Server responding to the request.  The    */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  This function returns zero if successful or a negative */
   /* return error code if there was an error.                          */
int _PBAM_Set_Phonebook_Response(unsigned int PBAPID, Byte_t ResponseCode);

   /* The following function sends a PBAP Pull vCard Listing Response   */
   /* to the specified remote PBAP Client.  This is used for responding */
   /* to a PBAP Pull vCard Listing Indication.  The PBAPID parameter    */
   /* specifies the PBAP ID of the PBAP Server responding to the        */
   /* request.  The ResponseCode parameter is the OBEX response code    */
   /* to include in the response.  The PhonebookSize parameter is a     */
   /* pointer to a variable that can optionally contain a Phonebook     */
   /* Size value to return in this request.  This should be done if     */
   /* the received indication indicated a request for PhonebookSize     */
   /* by indicating a MaxListCount = ZERO (0).  If this value is to     */
   /* be included in the response the Buffer parameter should be set    */
   /* to NULL and the BufferSize to ZERO.  If this value is NOT to be   */
   /* used in the response, this parameter should be set to NULL.  The  */
   /* NewMissedCalls parameter is a pointer to a variable that can      */
   /* optionally contain the number of new missed calls which have      */
   /* not been checked on this server.  This should only be included    */
   /* on requests for the 'mch' phonebook type.  If this value is to    */
   /* be included in the response the Buffer parameter should be set    */
   /* to NULL and the BufferSize to ZERO.  If this value is NOT to be   */
   /* used in the response, this parameter should be set to NULL.  The  */
   /* BufferSize parameter is the size in bytes of the data included    */
   /* in the specified Buffer.  The Buffer parameter is a pointer to a  */
   /* byte buffer containing the Phonebook listing data to be included  */
   /* in this response packet.  The AmountWritten parameter is a pointer*/
   /* to variable which will be written with the actual amount of data  */
   /* that was able to be included in the packet.  This function returns*/
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _PBAM_Pull_vCard_Listing_Response(unsigned int PBAPID, Byte_t ResponseCode, Word_t *PhonebookSize, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);

   /* The following function sends a PBAP Pull vCard Entry Response to  */
   /* the specified remote PBAP Client.  This is used for responding    */
   /* to a PBAP Pull vCard Entry Indication.  The PBAPID parameter      */
   /* specifies the PBAP ID of the PBAP Server responding to the        */
   /* request.  The ResponseCode parameter is the OBEX response code    */
   /* to include in the response.  The BufferSize parameter is the      */
   /* size in bytes of the data included in the specified Buffer.  The  */
   /* Buffer parameter is a pointer to a byte buffer containing the     */
   /* Phonebook entry data to be included in this response packet.      */
   /* The AmountWritten parameter is a pointer to variable which will   */
   /* be written with the actual amount of data that was able to be     */
   /* included in the packet.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
int _PBAM_Pull_vCard_Entry_Response(unsigned int PBAPID, Byte_t ResponseCode, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);

#endif
