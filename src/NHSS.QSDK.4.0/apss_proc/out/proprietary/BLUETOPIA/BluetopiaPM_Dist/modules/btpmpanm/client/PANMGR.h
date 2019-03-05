/*****< panmgr.h >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PANMGR - PAN Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/31/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __PANMGRH__
#define __PANMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPAN.h"            /* PAN Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PAN Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAN Manager  */
   /* Implementation.                                                   */
int _PANM_Initialize(void);

   /* The following function is responsible for shutting down the PAN   */
   /* Manager Implementation.  After this function is called the PAN    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PANM_Initialize() function.  */
void _PANM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming PAN connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A PAN Connection  */
   /*          Indication event will be dispatched to signify the actual*/
   /*          result.                                                  */
int _PANM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);

   /* The following function is provided to allow a mechanism for local */
   /* modules to open a remote PAN server port. This function returns   */
   /* zero if successful and a negative value if there was an error.    */
int _PANM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, unsigned long ConnectionFlags);

   /* The following function is provided to allow a mechanism for local */
   /* module to close a previously opened PAN Connection. This function */
   /* returns zero if successful, or a negative value if there was an   */
   /* error                                                             */
int _PANM_Close_Connection(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Personal*/
   /* Area Networking devices.  This function accepts the buffer        */
   /* information to receive any currently connected devices.  The first*/
   /* parameter specifies the maximum number of BD_ADDR entries that the*/
   /* buffer will support (i.e. can be copied into the buffer).  The    */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _PANM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for the Personal Area  */
   /* Networking (PAN) Manager.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
int _PANM_Query_Current_Configuration(PANM_Current_Configuration_t *CurrentConfiguration);

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the flags that control how incoming connections */
   /* are handled. This function returns zero if successful and a       */
   /* negative value if there was an error.                             */
int _PANM_Change_Incoming_Connection_Flags(unsigned int ConnectionFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register with the PAN Manager Server to receive PAN    */
   /* Event Notifications. This function returns a positive non-zero    */
   /* value if successful, or a negative value if there was an error.   */
   /* * NOTE * The value this function returns (if successful) can be   */
   /*          passed to the _PANM_Un_Register_Events() function to     */
   /*          un-register for events.                                  */
int _PANM_Register_Events(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to un-register from the server for receiving events.  This*/
   /* function accepts as input the EventCallbackID returned from       */
   /* _PANM_Register_Events() function.                                 */
int _PANM_Un_Register_Events(unsigned int PANEventsHandlerID);

#endif
