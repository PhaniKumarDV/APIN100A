/*****< btpmmapi.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMAPI - Main Entry Point API for Stonestreet One Bluetopia Platform    */
/*             Manager.                                                       */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/19/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMMAPIH__
#define __BTPMMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "DBGAPI.h"              /* BTPM Debug Module Prototypes/Constants.   */
#include "MSGAPI.h"              /* BTPM Message Module Prototypes/Constants. */
#include "DEVMAPI.h"             /* BTPM Device Manager Prototypes/Constants. */
#include "SETAPI.h"              /* BTPM Settings/Configuration Prototypes.   */
#include "SPPMAPI.h"             /* BTPM Serial Manager Prototypes/Constants. */

#if BTPM_CONFIGURATION_GENERIC_ATTRIBUTE_MANAGER_SUPPORTED

#include "GATMAPI.h"             /* BTPM GATT Manager Prototypes/Constants.   */

#endif

   /* The following structure is passed to the BTPM_Main() function to  */
   /* pass platform specific information to the individual modules that */
   /* the initialization function will initialize.                      */
typedef struct _tagBTPM_Initialization_Info_t
{
   BTPM_Debug_Initialization_Data_t *DebugInitializationInfo;
   SET_Initialization_Data_t        *SettingsInitializationInfo;
   MSG_Initialization_Data_t        *MessageInitializationInfo;
   DEVM_Initialization_Data_t       *DeviceManagerInitializationInfo;
   SPPM_Initialization_Data_t       *SerialPortManagerInitializationInfo;

#if BTPM_CONFIGURATION_GENERIC_ATTRIBUTE_MANAGER_SUPPORTED

   GATM_Initialization_Data_t       *GenericAttributeManagerInitializationInfo;

#endif

} BTPM_Initialization_Info_t;

#define BTPM_INITIALIZATION_INFO_SIZE                    (sizeof(BTPM_InitializationInfo_t))

   /* The following is a type definition of the User Supplied BTPM      */
   /* Callback function that can be scheduled to be called back from the*/
   /* main BTPM Handler.  This function is called in the main BTPM      */
   /* Handler Thread and NOT the thread that the Callback was           */
   /* established in.                                                   */
   /* ** NOTE ** The caller should keep the processing of these         */
   /*            Callbacks small because other Events will not be able  */
   /*            to be called while one is being serviced.              */
typedef void (BTPSAPI *BTPM_Dispatch_Callback_t)(void *CallbackParameter);

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Services (and beginning it's           */
   /* execution).  This function accepts as input, a pointer to an      */
   /* initialization container structure that contains various platform */
   /* specific initialization information for various portions of the   */
   /* Platform Manager.  The final two parameters specify a Dispatch    */
   /* Callback and Callback parameter (respectively) that is dispatched */
   /* when the Initialization procedure is complete.  The callback is   */
   /* optional, and if one is not required, should be specified as NULL.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error initializing the Bluetopia       */
   /* Platform Manager Services.                                        */
BTPSAPI_DECLARATION int BTPSAPI BTPM_Main(BTPM_Initialization_Info_t *InitializationInfo, BTPM_Dispatch_Callback_t InitializationDoneCallback, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_Main_t)(BTPM_Initialization_Info_t *InitializationInfo, BTPM_Dispatch_Callback_t InitializationDoneCallback, void *CallbackParameter);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to queue an Asynchronous Callback that will be dispatched  */
   /* from the Main BTPM Handler.  This function accepts the Callback   */
   /* Function and Callback Parameter (respectively) of the Callback    */
   /* that is being queued.  This function returns BOOLEAN TRUE if the  */
   /* callback was installed correctly, or BOOLEAN FALSE if the callback*/
   /* was unable to be installed.                                       */
BTPSAPI_DECLARATION Boolean_t BTPSAPI BTPM_QueueMailboxCallback(BTPM_Dispatch_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_BTPM_QueueMailboxCallback_t)(BTPM_Dispatch_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to set the current Debug Zone Mask that is used by the     */
   /* debugging Module.  This allows various levels of debug to be      */
   /* changed during run-time to control debugging output.  This        */
   /* function accepts as input the new Debug Zone Mask to set for the  */
   /* Control Server.  This function returns BOOLEAN TRUE if the Debug  */
   /* Zone Mask was set correctly, or BOOLEAN FALSE if the Debug Zone   */
   /* Mask was unable to be set.                                        */
BTPSAPI_DECLARATION Boolean_t BTPSAPI BTPM_SetDebugZoneMask(unsigned long DebugZoneMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_BTPM_SetDebugZoneMask_t)(unsigned long DebugZoneMask);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to query the the current Debug Zone Mask that is being used*/
   /* by the debugging Module.  This function returns the current Debug */
   /* Zone Mask that is currently being used.                           */
BTPSAPI_DECLARATION unsigned long BTPSAPI BTPM_QueryDebugZoneMask(unsigned int PageNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_BTPM_QueryDebugZoneMask_t)(unsigned int PageNumber);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to instruct the Bluetopia Platform Manager Services Server */
   /* to shutdown.  This function returns zero if the Bluetopia Platform*/
   /* Manager Services Server has been instructed to start shutting down*/
   /* or a negative return error code if there was an error.            */
BTPSAPI_DECLARATION int BTPSAPI BTPM_ShutdownService(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_ShutdownService_t)(void);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to set the current Debug Zone Mask that is used by the     */
   /* remote Process's Client debugging Module.  This allows various    */
   /* levels of debug to be changed during run-time to control debugging*/
   /* output.  This function accepts as input the Process ID of the     */
   /* Client Process that is to have it's Debug Zone Mask changed       */
   /* followed by the new Debug Zone Mask to set for the Client Library */
   /* (specified by the first parameter).  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI BTPM_SetDebugZoneMaskPID(unsigned long ProcessID, unsigned long DebugZoneMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_SetDebugZoneMaskPID_t)(unsigned long ProcessID, unsigned long DebugZoneMask);
#endif

#endif
