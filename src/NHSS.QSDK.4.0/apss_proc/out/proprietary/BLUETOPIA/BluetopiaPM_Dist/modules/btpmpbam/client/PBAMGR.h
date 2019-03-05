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
int _PBAM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Phone Book Access Manager implementation. After this function     */
   /* is called the Phone Book Access Manager implementation will no    */
   /* longer operate until it is initialized again via a call to the    */
   /* _PBAM_Initialize() function.                                      */
void _PBAM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Phone Book Access device. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error. This function accepts the connection  */
   /* information for the remote device (address and server port). This */
   /* function accepts the connection flags to apply to control how the */
   /* connection is made regarding encryption and/or authentication.    */
int _PBAM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags);

   /* The following function exists to close an active Phone Book Access*/
   /* connection that was previously opened by a Successful call to     */
   /* _PBAM_Connect_Remote_Device() function.  This function accepts as */
   /* input the device address of the local connection which should     */
   /* close its active connection.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
int _PBAM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding PBAP Profile Client Request.  This function accepts   */
   /* the device address of the device that is to have the PBAP         */
   /* operation aborted.  This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
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
int _PBAM_Abort_Request(BD_ADDR_t RemoteDeviceAddress);

   /* The following function generates a PBAP Pull Phone Book request to*/
   /* the specified remote PBAP Server.  The first parameter specifies  */
   /* the remote, connected, device to issue the request on.  The       */
   /* PhoneBookNamePath parameter contains the name/path of the phone   */
   /* book being requested by this pull phone book operation.  This     */
   /* value can be NULL if a phone book size request is being performed.*/
   /* The FilterLow parameter contains the lower 32 bits of the 64-bit  */
   /* filter attribute.  The FilterHigh parameter contains the higher 32*/
   /* bits of the 64-bit filter attribute.  The Format parameter is an  */
   /* enumeration which specifies the vCard format requested in this    */
   /* pull phone book request.  If pfDefault is specified then the      */
   /* format will not be included in the request (note that the server  */
   /* will default to pfvCard21 in this case).  The MaxListCount        */
   /* parameter is an unsigned integer that specifies the maximum number*/
   /* of entries the client can handle.  A value of 65535 means that the*/
   /* number of entries is not restricted.  A MaxListCount of ZERO (0)  */
   /* indicates that this is a request for the number of used indexes in*/
   /* the Phonebook specified by the PhoneBookNamePath parameter.  The  */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this PullPhonebookRequest.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP profile server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP profile server successfully */
   /*          executed the request.                                    */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current request is completed (this is   */
   /*          signified by receiving a confirmation event in the PBAP  */
   /*          profile event callback that was registered when the PBAM */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_Phone_Book_Request(BD_ADDR_t RemoteDeviceAddress, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t Format, Word_t MaxListCount, Word_t ListStartOffset);

   /* The following function generates a PBAP Pull Phone Book request to*/
   /* the specified remote PBAP server to request the size of the remote*/
   /* phone book. The Bluetooth Address parameter specifies the PBAP    */
   /* client. This function returns zero if successful or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the _PBAM_Abort_Request() */
   /*          function) or the current request is completed.           */
int _PBAM_Pull_Phone_Book_Size(BD_ADDR_t RemoteDeviceAddress);

   /* The following function generates a PBAP Set Phonebook Request to  */
   /* the specified remote PBAP Server.  The first parameter specifies  */
   /* the remote, connected, device address of the device to issue the  */
   /* request on.  The PathOption parameter contains an enumerated value*/
   /* that indicates the type of path change to request.  The FolderName*/
   /* parameter contains the folder name to include with this Set       */
   /* Phonebook request.  This value can be NULL if no name is required */
   /* for the selected PathOption.  See the PBAP specification for more */
   /* information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
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
int _PBAM_Set_Phone_Book_Request(BD_ADDR_t RemoteDeviceAddress, PBAM_Set_Path_Option_t PathOption, char *FolderName);

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
int _PBAM_Pull_vCard_Listing(BD_ADDR_t RemoteDeviceAddress, char *PhonebookPath, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset);

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
int _PBAM_Pull_vCard(BD_ADDR_t RemoteDeviceAddress, char *VCardName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t VCardFormat);

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
int _PBAM_Set_Phone_Book_Absolute(BD_ADDR_t RemoteDeviceAddress, char *Path);

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
int _PBAM_Register_Server(unsigned int ServerPort, unsigned int SupportedRepositories, unsigned long IncomingConnectionFlags, char *ServiceName);

   /* Unregisters a previously opened PBAP server port. The ServerID    */
   /* parameter is the ID of the server returned from a successful      */
   /* call to _PBAM_Register_Server(). This fuction returns zero if     */
   /* successful or a negative return error code if there was an error. */
int _PBAM_Un_Register_Server(unsigned int ServerID);

   /* Respond to an outstanding connection request to the local         */
   /* server. The ConnectionID is the indentifier of the connection     */
   /* request returned in a petConnectionRequest event. The Accept      */
   /* parameter indicates whether the connection should be accepted or  */
   /* rejected. This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _PBAM_Connection_Request_Response(unsigned int ConnectionID, Boolean_t Accept);

   /* Close an active connection to a local PBAP Server instance. The   */
   /* ConnectionID parameter is the identifier of the connection        */
   /* returned in a petConnected event. This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _PBAM_Close_Server_Connection(unsigned int ConnectionID);

   /* Submits a response to a received etPullPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the active connection */
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes. If the request */
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
int _PBAM_Send_Phone_Book(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

   /* Submits a response to a received etPullPhoneBookSize event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode          */
   /* parameter is one of the defined _PBAM Response Status codes. The  */
   /* PhonebookSize parameter indicates the number of entries in the    */
   /* requested phone book. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int _PBAM_Send_Phone_Book_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int PhoneBookSize);

   /* Submits a response to a received petSetPhoneBook event. The       */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes. This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PBAM_Set_Phone_Book_Response(unsigned int ConnectionID, unsigned int ResponseStatusCode);

   /* Submits a response to a received petPullvCardListing event. The   */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes.  If the request*/
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
int _PBAM_Send_vCard_Listing(unsigned int ConnectionID, unsigned int ResponseStatusCode, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

   /* Submits a response to a received petPullvCardListingSize          */
   /* event. The ConnectionID parameter is the identifier of the        */
   /* activer connection returned in a petConnected event. The          */
   /* ResponseStatusCode parameter is one of the defined _PBAM Response */
   /* Status codes. The vCardListingSize parameter indicates the number */
   /* of vCard entries in the current/specfied folder. This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PBAM_Send_vCard_Listing_Size(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int vCardListingSize);

   /* Submits a response to a received petPullvCard event. The          */
   /* ConnectionID parameter is the identifier of the activer connection*/
   /* returned in a petConnected event. The ResponseStatusCode parameter*/
   /* is one of the defined _PBAM Response Status codes.  The BufferSize*/
   /* parameter indicates the amount of data in the buffer to be        */
   /* sent. The Buffer parameter is a pointer to the vCard data to      */
   /* send. The Final parameter should be set to FALSE if there is more */
   /* data to be sent after this buffer or TRUE if there is no more     */
   /* data. This function returns zero if successful and a negative     */
   /* return error code if there was an error.                          */
int _PBAM_Send_vCard(unsigned int ConnectionID, unsigned int ResponseStatusCode, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final);

#endif
