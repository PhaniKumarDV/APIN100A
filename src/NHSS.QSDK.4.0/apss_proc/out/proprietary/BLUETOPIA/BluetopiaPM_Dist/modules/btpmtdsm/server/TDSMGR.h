/*****< tdsmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSMGR - 3D Sync Manager Implementation for Stonestreet One Bluetooth     */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __TDSMGRH__
#define __TDSMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTTDSM.h"           /* TDS Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the 3D Sync Manager implementation.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* 3D Sync Manager Implementation.                                   */
int _TDSM_Initialize(TDSM_Initialization_Info_t *TDSInitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* 3D Sync Manager implementation.  After this function is called the*/
   /* 3D Sync Manager implementation will no longer operate until it is */
   /* initialized again via a call to the _TDSM_Initialize() function.  */
void _TDSM_Cleanup(void);

   /* The following function is responsible for informing the 3D Sync   */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the 3D Sync Manager with the    */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _TDSM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  The SyncTrainParams      */
   /* parameter is a pointer to the Synchronization Train parameters to */
   /* be set.  The IntervalResult parameter is a pointer to the variable*/
   /* that will be populated with the Synchronization Interval chosen   */
   /* by the Bluetooth controller.  This function will return zero on   */
   /* success; otherwise, a negative error value will be returned.      */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
int _TDSM_Write_Synchronization_Train_Parameters(TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult);

   /* The following function will enable the Synchronization Train.     */
   /* This function will return zero on success; otherwise, a negative  */
   /* error value will be returned.                                     */
   /* * NOTE * The TDSM_Write_Synchronization_Train_Parameters function */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The tetTDSM_Display_Synchronization_Train_Complete event */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
int _TDSM_Start_Synchronization_Train(void);

   /* The following function will configure and enable the              */
   /* Connectionless Slave Broadcast channel on the Bluetooth           */
   /* controller.  The ConnectionlessSlaveBroadcastParams parameter is  */
   /* a pointer to the Connectionless Slave Broadcast parameters to be  */
   /* set.  The IntervalResult parameter is a pointer to the variable   */
   /* that will be populated with the Broadcast Interval chosen by the  */
   /* Bluetooth controller.  This function will return zero on success; */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
int _TDSM_Enable_Connectionless_Slave_Broadcast(TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult);

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.                           */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
int _TDSM_Disable_Connectionless_Slave_Broadcast(void);

   /* The following function is used to get the current information     */
   /* being used int the synchronization broadcasts.  The               */
   /* CurrentBroadcastInformation parameter to a structure in which the */
   /* current information will be placed.  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
int _TDSM_Get_Current_Broadcast_Information(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation);

   /* The following function is used to update the information          */
   /* being sent in the synchronization broadcasts.  The                */
   /* BroadcastInformationUpdate parameter is a pointer to a structure  */
   /* which contains the information to update.  This function returns  */
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
int _TDSM_Update_Broadcast_Information(TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate);

#endif
