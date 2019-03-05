/*****< panmgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
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

#include "SS1BTPANM.h"           /* PAN Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PAN Manager Implementation. This function returns  */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAN Manager  */
   /* Implementation.                                                   */
int _PANM_Initialize(PANM_Initialization_Info_t *PANMInitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* PAN Manager Implementation. After this function is called the     */
   /* PAN Manager Implementation will no longer operate until it is     */
   /* initialized again via a call to the _PANM_Initialize() function.  */
void _PANM_Cleanup(void);

   /* The following function is responsible for informing the PAN       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the PAN Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _PANM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to accept or reject a request to open a connection to     */
   /* the local Server. This function returns zero if successful and a  */
   /* negative value if there was an error.                             */
int _PANM_Open_Request_Response(unsigned int PANID, Boolean_t AcceptConnection);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to opena remote PAN Server. This function returns   */
   /* a positive non-zero value representing the PANID of the opened    */
   /* connection is successful, and a negative value if there was an    */
   /* error.                                                            */
int _PANM_Open_Remote_Server(BD_ADDR_t BD_ADDR, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType);

   /* The following function is provided to allow a mechanism for local */
   /* modules to close a previously opened PAN Connection. This function*/
   /* returns zero if successful and a negative value if there was an   */
   /* error.                                                            */
   /* * NOTE * This function does NOT un-register any server. It only   */
   /*          closes any active connections.                           */
int _PANM_Close_Connection(unsigned int PANID);

#endif
