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

   /* The following macros are used for manipulating "Instance" values, */
   /* which are used to represent HDP instances on remote devices by    */
   /* encoding the Control/Data PSM pair.                               */
#define SET_INSTANCE(_CONTROL_PSM, _DATA_PSM) (((_CONTROL_PSM) << 16) + ((_DATA_PSM) & 0x0000FFFF))

#define GET_CONTROL_PSM(_INSTANCE) (((_INSTANCE) >> 16) & 0x0000FFFF)

#define GET_DATA_PSM(_INSTANCE)    ((_INSTANCE) & 0x0000FFFF)

#define VALID_INSTANCE(__INSTANCE) ((Boolean_t)(((L2CAP_PSM_VALID_PSM(GET_CONTROL_PSM(__INSTANCE))) && (L2CAP_PSM_VALID_PSM(GET_DATA_PSM(__INSTANCE))))))

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Health Device Manager implementation.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Health Device Manager Implementation.                     */
int _HDPM_Initialize(HDPM_Initialization_Info_t *HDPInitializationInfo);

   /* The following function is responsible for shutting down the Hands */
   /* Free Manager implementation.  After this function is called the   */
   /* Health Device Manager implementation will no longer operate until */
   /* it is initialized again via a call to the _HDPM_Initialize()      */
   /* function.                                                         */
void _HDPM_Cleanup(void);

   /* The following function is responsible for informing the Health    */
   /* Device Manager Implementation of the currently opened Bluetooth   */
   /* stack's ID.                                                       */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Health Device Manager with  */
   /*          the specified Bluetooth Stack ID.  When this parameter   */
   /*          is set to zero, this function will actually clean up all */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HDPM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* module to register an endpoint on the local HDP server instance.  */
   /* This function will return a positive, non-zero value on success,  */
   /* which represents the MDEP_ID of the new endpoint. A negative error*/
   /* code is returned on failure.                                      */
int _HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t Role, char *Description);

   /* The following function is provided to allow a mechanism for       */
   /* local module to un-register an endpoint on the local HDP server   */
   /* instance. This function returns zero value on success, or a       */
   /* negative error code on failure.                                   */
int _HDPM_Un_Register_Endpoint(Byte_t MDEP_ID, Word_t DataType, HDP_Device_Role_t Role);

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to an incoming Data Channel connection request. */
   /* This function returns zero value on success, or a negative error  */
   /* code on failure.                                                  */
int _HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to initiate a connection to an HDP instance on a     */
   /* remote device. The function returns a positive, non-zero value    */
   /* representing the MCLID of the new connection, if successful, or a */
   /* negative error code.                                              */
int _HDPM_Connect_Remote_Instance(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to close an existing connection to an HDP instance   */
   /* on a remote device. The function returns zero if successful, or a */
   /* negative error code.                                              */
int _HDPM_Disconnect_Remote_Instance(unsigned int MCLID);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to initiate a data channel connection to an HDP      */
   /* instance on a remote device. The function returns a positive,     */
   /* non-zero value representing the DataLinkID of the new connection, */
   /* if successful, or a negative error code.                          */
int _HDPM_Connect_Data_Channel(unsigned int MCLID, Byte_t MDEP_ID, HDP_Device_Role_t Role, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to disconnect an existing HDP data channel. The      */
   /* function returns zero if successful, or a negative error code.    */
int _HDPM_Disconnect_Data_Channel(unsigned int MCLID, unsigned int DataLinkID);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to write data to an existing HDP data channel. The   */
   /* function returns zero if successful, or a negative error code.    */
int _HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to send a response to a Sync Capabilities request.   */
   /* The function returns zero if successful, or a negative error code.*/
int _HDPM_Sync_Capabilities_Response(unsigned int MCLID, Byte_t AccessResolution, Word_t SyncLeadTime, Word_t NativeResolution, Word_t NativeAccuracy, Byte_t ResponseCode);

//XXX

#endif
