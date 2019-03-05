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

   /* Initializes the platform manager client Object Push Manager       */
   /* implementation.  This function returns zero if successful, or a   */
   /* negative integer if there was an error.                           */
int _OPPM_Initialize(void);

   /* This function sets the pm client Initialized global flag to false.*/
   /* The other methods will fail until _OPPM_Initialize has been       */
   /* called.                                                           */
void _OPPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming OPP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A OPP Connected   */
   /*          event will be dispatched to signify the actual result.   */
int _OPPM_Connection_Request_Response(unsigned int ServerID, Boolean_t Accept);

   /* The following function is provided to allows a mechanism for local*/
   /* modules to register an Object Push Server. This first parameter   */
   /* is the RFCOMM server port. If this parameter is zero, the server  */
   /* will be opened on an available port. The SupportedObjectTypes     */
   /* parameter is a bitmask representing the types of objects supported*/
   /* by this server. The IncomingConnectionFlags parameter is a        */
   /* bitmask which indicates whether incoming connections should       */
   /* be authorized, autenticated, or encrypted. The ServiceName        */
   /* parameter is a null-terminate string represting the name of the   */
   /* service to be placed in the Service Record. The EventCallback     */
   /* is the function which will receive events related to this         */
   /* server. The CallbackParameter will be included in each call to    */
   /* the CallbackFunction. This function returns a positive integer    */
   /* representing the ServerID of the created server if successful and */
   /* a negative error code if there was an error.                      */
int _OPPM_Register_Server(unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName);

   /* The following function is provided to allows a mechanism for      */
   /* local modules to register an Object Push Server registered by a   */
   /* successful call to OPPM_Register_Server(). This function accepts  */
   /* as a parameter the ServerID returned from a successful call to    */
   /* OPPM_Register_Server(). This function returns zero if successful  */
   /* and a negative error code if there was an error.                  */
int _OPPM_Un_Register_Server(unsigned int ServerID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Object Push Server device.  The    */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The ConnectionFlags*/
   /* parameter specifies whether authentication or encryption should   */
   /* be used to create this connection.  The CallbackFunction is the   */
   /* function that will be registered to receive events for this       */
   /* connection.  The CallbackParameter is a parameter which will be   */
   /* included in the status callback.  This function returns zero if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Message Access Manager Event Callback supplied.          */
int _OPPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags);

   /* The following function exists to close an active Object Push      */
   /* connection that was previously opened by a successful call to     */
   /* OPPM_Connect_Remote_Device() function or by a oetConnected        */
   /* event. This function accpets the either the ClientID or ServerID  */
   /* of the connection as a parameter. This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
int _OPPM_Disconnect(unsigned int PortID);

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding OPPM profile client request.  This function accepts as*/
   /* input the ClientID of the device specifying which connection is to*/
   /* have the Abort issued.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int _OPPM_Abort(unsigned int ClientID);

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter    */
   /* is the ClientID of the remote device connection. The ObjectType   */
   /* parameter specifies the type of object being pushed. The Object   */
   /* Name parameter is a UNICODE encoded string representing the name  */
   /* of the object to push. The DataLength and DataBuffer specify the  */
   /* length and contents of the object. This function returns zero if  */
   /* successful and a negative error code if there is an error.        */
   /* * NOTE * The Object Name is a pointer to a NULL Terminated        */
   /*          UNICODE String.                                          */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _OPPM_Push_Object_Request(unsigned int ClientID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

   /* The following function is responsible for sending an Object       */
   /* Push Response to the remote Client.  The first parameter is       */
   /* the ServerID of the local Object Push server. The ResponseCode    */
   /* parameter is the OPPM response status code associated with this   */
   /* response. The function returns zero if successful and a negative  */
   /* error code if there is an error.                                  */
int _OPPM_Push_Object_Response(unsigned int ServerID, unsigned int ResponseCode);

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The Client        */
   /* parameter is the ClientID of the remote Object Push server        */
   /* connection. This function returns zero if successful and a        */
   /* negative error code if there was an error.                        */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int _OPPM_Pull_Business_Card_Request(unsigned int ClientID);

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter is the   */
   /* ServerID of the remote Object Push client. The ResponseCode       */
   /* parameter is the OPPM response status code associated with the    */
   /* response. The DataLength and DataBuffer parameters contain the    */
   /* business card data to be sent. This function returns zero if      */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int _OPPM_Pull_Business_Card_Response(unsigned int ServerID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

#endif
