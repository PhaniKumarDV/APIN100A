/*****< oppmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPMGR - Object Push Profile Manager Implementation for Stonestreet One   */
/*          Bluetooth Protocol Stack Platform Manager.                        */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/11/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __OPPMGRH__
#define __OPPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTOPPM.h"           /* OPP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Object Push Manager implementation.  This function */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Object Push Manager implementation.                               */
int _OPPM_Initialize(void);

   /* The following function is responsible for shutting down the Object*/
   /* Push Manager implementation.  After this function is called the   */
   /* Object Push Manager implementation will no longer operate until   */
   /* it is initialized again via a call to the _OPPM_Initialize()      */
   /* function.                                                         */
void _OPPM_Cleanup(void);

   /* The following function is responsible for informing the Object    */
   /* Push Manager implementation of that Bluetooth stack ID of the     */
   /* currently opened Bluetooth stack.  When this parameter is set     */
   /* to non-zero, this function will actually initialize the Object    */
   /* Push Manager with the specified Bluetooth stack ID.  When this    */
   /* parameter is set to zero, this function will actually clean up all*/
   /* resources associated with the prior initialized Bluetooth Stack.  */
void _OPPM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is responsible for opening a local OPP     */
   /* Server.  The parameter to this function is the Port on which to   */
   /* open this server, and *MUST* be between OPP_PORT_NUMBER_MINIMUM   */
   /* and OPP_PORT_NUMBER_MAXIMUM.  This function returns a positive,   */
   /* non zero value if successful or a negative return error code if an*/
   /* error occurs.  A successful return code will be a OPP Profile ID  */
   /* that can be used to reference the Opened OPP Profile Server Port  */
   /* in ALL other OPP Server functions in this module.  Once an OPP    */
   /* Profile Server is opened, it can only be Un-Registered via a call */
   /* to the _OPP_Close_Server() function (passing the return value from*/
   /* this function).                                                   */
int _OPPM_Open_Object_Push_Server(unsigned int ServerPort);

   /* The following function is responsible for closing a               */
   /* currently open/registered Object Push Profile server.  This       */
   /* function is capable of closing servers opened via a call to       */
   /* _OPPM_Open_Object_Oush_Server().  The parameter to this function  */
   /* is the OPPM ID of the Profile Server to be closed.  This function */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
int _OPPM_Close_Server(unsigned int OPPID);

   /* The following function adds a OPP Server (MSE) Service Record to  */
   /* the SDP Database.  The first parameter is the OPP ID that was     */
   /* returned by a previous call to _OPP_Open_Object_Push_Server().    */
   /* The second parameter is a pointer to ASCII, NULL terminated string*/
   /* containing the Service Name to include within the SDP Record.     */
   /* The third parameter is the Supported Object Type Bitmask value.   */
   /* The final parameter will be set to the service record handle on   */
   /* success.  This function returns zero on success or a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * This function should only be called with the OPP ID that */
   /*          was returned from the _OPP_Open_Object_Push_Server()     */
   /*          function.  This function should NEVER                    */
   /*          be used with OPP ID returned from the                    */
   /*          _OPP_Open_Remote_Object_Push_Server_Port() function.     */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the _OPP_Un_Register_SDP_Record()  */
   /*          function.                                                */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
int _OPPM_Register_Object_Push_Server_SDP_Record(unsigned int OPPID, char *ServiceName, DWord_t SupportedObjectTypes, DWord_t *RecordHandle);

   /* The following function is responsible for deleting a previously   */
   /* registered OPP SDP Service Record.  This function accepts as its  */
   /* parameter the SDP Service Record Handle of the SDP Service Record */
   /* to delete from the SDP Database.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _OPPM_Un_Register_SDP_Record(unsigned int OPPID, DWord_t ServiceRecordHandle);

   /* The following function is responsible for opening a connection    */
   /* from a local Object Push Client to a remote Object Push Server.   */
   /* The first parameter to this function is the Bluetooth Stack ID of */
   /* the Bluetooth Protocol Stack Instance to be associated with this  */
   /* Object Push Profile connection.  The second parameter to this     */
   /* function is the BD_ADDR of the remote Object Push Server in which */
   /* to connect.  The third parameter to this function is the Remote   */
   /* Server Port where the Push Server is registered.  The fourth and  */
   /* fifth parameters are the Event Callback function and application  */
   /* defined Callback Parameter to be used when OPP Events occur.  This*/
   /* function returns a non-zero, positive, number on success or a     */
   /* negative return value if there was an error.  A successful return */
   /* value will be a OPP ID that can used to reference the Opened      */
   /* Object Push Profile connection to a remote Object Push Server in  */
   /* ALL other applicable functions in this module.                    */
   /* ** NOTE ** The Object Push Server Port value must be specified    */
   /*            and must be a value between OPP_PORT_NUMBER_MINIMUM and*/
   /*            OPP_PORT_NUMBER_MAXIMUM.                               */
int _OPPM_Open_Remote_Object_Push_Server(BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort);

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Object Push Profile      */
   /* Server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Stack associated with the Object Push   */
   /* Profile Server that is responding to the request.  The second     */
   /* parameter to this function is the OPP ID of the Object Push       */
   /* Profile for which a connection request was received.  The final   */
   /* parameter to this function specifies whether to accept the pending*/
   /* connection request (or to reject the request).  This function     */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etOPP_Open_Imaging_Service_Port_Indication event has   */
   /*            occurred.                                              */
int _OPPM_Open_Request_Response(unsigned int OPPID, Boolean_t AcceptConnection);

   /* The following function is responsible for closing a currently     */
   /* on-going Object Push Profile connection.  The first parameter to  */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Object Push Profile    */
   /* connection being closed.  The second parameter to this function is*/
   /* the OPP ID of the Object Push Profile connection to be closed.    */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** If this function is called with a server OPP ID (value */
   /*            returned from OPP_Open_Object_Push_Server()) any       */
   /*            clients current connection to this server will be      */
   /*            terminated, but the server will remained registered.   */
   /*            If this function is call using a client OPP ID (value  */
   /*            returned from OPP_Open_Remote_Object_Push_Server()) the*/
   /*            client connection shall be terminated.                 */
int _OPPM_Close_Connection(unsigned int OPPID);

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Object Push Server.  The first parameter to this    */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Client   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Client making this call.  This  */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on-going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
int _OPPM_Abort_Request(unsigned int OPPID);

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter to */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Object Push Client     */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Client making this call.  The third     */
   /* parameter to this function is the actual type of object that is   */
   /* being pushed.  The fourth parameter to this function is the Name  */
   /* of the Object being put (in NULL terminated UNICODE format).  The */
   /* fifth parameter is the total length of the object being put.  The */
   /* sixth and seventh parameters to this function specify the length  */
   /* of the Object Data and a pointer to the Object Data being Pushed. */
   /* The eighth parameter to this function is a pointer to a length    */
   /* variable that will receive the total amount of data actually sent */
   /* in the request.  The final parameter to this function is a Boolean*/
   /* Flag indicating if this is to be the final segment of the object. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** This function should be used to initiate the Push      */
   /*            Object transaction as well as to continue a previously */
   /*            initiated, on-going, Push Object transaction.          */
   /* ** NOTE ** The Object Name is a pointer to a NULL Terminated      */
   /*            UNICODE String.                                        */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Object Push Profile function.     */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Object Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            not all parameters to this function are used in each   */
   /*            segment of the transaction.                            */
int _OPPM_Push_Object_Request(unsigned int OPPID, OPP_Object_Type_t ObjectType, Word_t *ObjectName, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);

   /* The following function is responsible for sending an Object Push  */
   /* Response to the remote Client.  The first parameter to this       */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Server making this call.  The   */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int _OPPM_Push_Object_Response(unsigned int OPPID, Byte_t ResponseCode);

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The first         */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Object Push Profile Client making this call.  The second parameter*/
   /* to this function is the OPP ID of the Object Push Client making   */
   /* this call.  This function returns zero if successful, or a        */
   /* negative return value if there was an error.                      */
   /* ** NOTE ** This function should be used to initiate the Pull      */
   /*            Business Card transaction as well as to continue a     */
   /*            previously initiated, on-going, Pull Business Card     */
   /*            transaction.                                           */
int _OPPM_Pull_Business_Card_Request(unsigned int OPPID);

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter to this  */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Server making this call.  The   */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth parameter specifies the*/
   /* Total Length of the Business card being pulled.  The fifth and    */
   /* sixth parameters to this function specify the length of the data  */
   /* being sent and a pointer to the data being sent.  The seventh     */
   /* parameter to this function is a pointer to a length variable that */
   /* will receive the amount of data actually sent.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** If the value returned in AmountWritten is less than the*/
   /*            value specified in DataLength, then an additional call */
   /*            to this function must be made to send the remaining    */
   /*            data.  Note that an additional call cannot be made     */
   /*            until AFTER another Pull Business Card Request Event is*/
   /*            received.                                              */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Object Push Profile function.     */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Object Total       */
   /*            Length), others don't appear in the first segment but  */
   /*            may appear in later segments (i.e. Body).  This being  */
   /*            the case, not all parameters to this function are used */
   /*            in each segment of the transaction.                    */
int _OPPM_Pull_Business_Card_Response(unsigned int OPPID, Byte_t ResponseCode, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten);

#endif
