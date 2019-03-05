/*****< hdpmgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDPMGR - Health Device Profile Manager Implementation for Stonestreet One */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HDPMGRH__
#define __HDPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHDPM.h"           /* HDP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HDP Manager implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HDP Manager  */
   /* Implementation.                                                   */
int _HDPM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* HDP Manager implementation.  After this function is called the    */
   /* HDP Manager implementation will no longer operate until it is     */
   /* initialized again via a call to the _HDPM_Initialize() function.  */
void _HDPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register an Endpoint on the Local HDP Server. The first*/
   /* parameter defines the Data Type that will be supported by this    */
   /* endpoint. The second parameter specifies whether the Endpoint     */
   /* will be a data source or sink. The third parameter is optional    */
   /* and can be used to specify a short, human-readable description of */
   /* the Endpoint. The final parameters specify the Event Callback and */
   /* Callback parameter (to receive events related to the registered   */
   /* endpoint). This function returns a positive, non-zero, value if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value represents the Endpoint ID that*/
   /*          can be used with various functions in this module to     */
   /*          refer to this endpoint.                                  */
int _HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t LocalRole, char *Description);

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Endpoint. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _HDPM_Un_Register_Endpoint(unsigned int EndpointID);

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to establish a data connection to a*/
   /* local endpoint. The first parameter is the DataLinkID associated  */
   /* with the connection request. The second parameter is one of       */
   /* the MCAP_RESPONSE_CODE_* constants which indicates either that    */
   /* the request should be accepted (MCAP_RESPONSE_CODE_SUCCESS) or    */
   /* provides a reason for rejecting the request. If the request is to */
   /* be accepted, and the request is for a local Data Source, the final*/
   /* parameter indicates whether the connection shall use the Reliable */
   /* or Streaming communication mode. This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Data Connected  */
   /*          event will be dispatched to signify the actual result.   */
   /* * NOTE * If the connection is accepted, and the connection request*/
   /*          is for a local Data Sink, then ChannelMode must be set to*/
   /*          the Mode indicated in the request.  If the connection is */
   /*          accepted for a local Data Source, ChannelMode must be set*/
   /*          to either cmReliable or cmStreaming. If the connection   */
   /*          request is rejected, ChannelMode is ignored.             */
int _HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to query the available HDP Instances on a remote    */
   /* device. The first parameter specifies the Address of the Remote   */
   /* Device to query. The second parameter specifies the maximum       */
   /* number of Instances that the buffer will support (i.e. can be     */
   /* copied into the buffer). The next parameter is optional and,      */
   /* if specified, will be populated with up to the total number of    */
   /* Instances advertised by the remote device, if the function is     */
   /* successful. The final parameter is optional and can be used to    */
   /* retrieve the total number of available Instances (regardless of   */
   /* the size of the list specified by the first two parameters).      */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Instances that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Instance  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int _HDPM_Query_Remote_Device_Instances(BD_ADDR_t RemoteDeviceAddress, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the available Endpoints published for a specific */
   /* HDP Instances on a remote device. The first parameter specifies   */
   /* the Address of the Remote Device to query. The second parameter   */
   /* specifies Instance on the Remote Device. The third parameter      */
   /* specifies the maximum number of Endpoints that the buffer will    */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with up to the   */
   /* total number of Endpoints published by the remote device, if the  */
   /* function is successful. The final parameter is optional and can   */
   /* be used to retrieve the total number of Endpoints (regardless     */
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Endpoints that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Endpoint  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int _HDPM_Query_Remote_Device_Instance_Endpoints(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the description of a known Endpoint published in */
   /* a specific HDP Instance by a remote device. The first parameter   */
   /* specifies the Address of the Remote Device to query. The second   */
   /* parameter specifies Instance on the Remote Device. The third      */
   /* parameter identifies the Endpoint to query. The fourth and fifth  */
   /* parameters specific the size of the buffer and the buffer to hold */
   /* the description string, respectively. The final parameter is      */
   /* optional and, if specified, will be set to the total size of the  */
   /* description string for the given Endpoint, if the function is     */
   /* successful (regardless of the size of the list specified by the   */
   /* first two parameters). This function returns a non-negative value */
   /* if successful which represents the number of bytes copied into the*/
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum           */
   /*          Description Length, in which case the final parameter    */
   /*          *MUST* be specified.                                     */
int _HDPM_Query_Remote_Device_Endpoint_Description(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Endpoint_Info_t *EndpointInfo, unsigned int MaximumDescriptionLength, char *DescriptionBuffer, unsigned int *TotalDescriptionLength);

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a connection to a specific HDP Instance on a */
   /* Remote Device. The first parameter specifies the Remote Device to */
   /* connect to. The second parameter specifies the HDP Instance on the*/
   /* remote device. The final parameters specify the Event Callback and*/
   /* Callback parameter (to receive events related to the connection   */
   /* attempt). This function returns zero if successful, or a negative */
   /* return value if there was an error.                               */
int _HDPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance);

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an existing connection to a specific HDP Instance*/
   /* on a Remote Device. The first parameter specifies the Remote      */
   /* Device. The second parameter specifies the HDP Instance on the    */
   /* remote device from which to disconnect. This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
int _HDPM_Disconnect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to establish an HDP connection to an Endpoint of    */
   /* a specific HDP Instance on a Remote Device. The first parameter   */
   /* specifies the Remote Device to connect to. The second parameter   */
   /* specifies the HDP Instance on the remote device. The third        */
   /* parameter specifies the Endpoint of that Instance to which the    */
   /* connection will be attempted. The fourth parameter specifies the  */
   /* type of connection that will be established. The final parameters */
   /* specify the Event Callback and Callback parameter (to receive     */
   /* events related to the connection). This function returns a        */
   /* positive value if successful, or a negative return value if there */
   /* was an error.                                                     */
   /* * NOTE * A successful return value represents the Data Link ID    */
   /*          shall be used with various functions and by various      */
   /*          events in this module to reference this data connection. */
int _HDPM_Connect_Remote_Device_Endpoint(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, Byte_t EndpointID, HDP_Channel_Mode_t ChannelMode);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect an established HDP data connection.   */
   /* This function accepts the Data Link ID of the data connection     */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
int _HDPM_Disconnect_Remote_Device_Endpoint(unsigned int DataLinkID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send data over an established HDP data connection. The */
   /* first parameter is the Data Link ID which represents the data     */
   /* connection to use. The final parameters specify the data (and     */
   /* amount) to be sent. This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function will either send all of the data or none of*/
   /*          the data.                                                */
int _HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer);

#endif

