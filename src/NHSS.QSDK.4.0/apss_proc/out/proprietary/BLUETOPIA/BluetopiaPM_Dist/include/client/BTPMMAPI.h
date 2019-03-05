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

#include "DBGAPI.h"              /* BTPM Debug Module Prototypes/Constants.   */
#include "MSGAPI.h"              /* BTPM Message Module Prototypes/Constants. */
#include "DEVMAPI.h"             /* BTPM Device Manager Prototypes/Constants. */
#include "SETAPI.h"              /* BTPM Settings/Configuration Prototypes.   */

   /* The following structure is passed to the BTPM_Main() function to  */
   /* pass platform specific information to the individual modules that */
   /* the initialization function will initialize.                      */
typedef struct _tagBTPM_Initialization_Info_t
{
   BTPM_Debug_Initialization_Data_t *DebugInitializationInfo;
   SET_Initialization_Data_t        *SettingsInitializationInfo;
   MSG_Initialization_Data_t        *MessageInitializationInfo;
} BTPM_Initialization_Info_t;

#define BTPM_INITIALIZATION_INFO_SIZE                    (sizeof(BTPM_InitializationInfo_t))

   /* The following is a type definition of the User Supplied BTPM      */
   /* Callback function that can be installed when the BTPM_Initialize()*/
   /* function is called.  This callback will be called when this module*/
   /* detects that the server is no longer registered or present in the */
   /* system.  This function is called in an arbitrary BTPM Handler     */
   /* Thread and NOT the thread that the Callback was established in.   */
   /* ** NOTE ** The caller should keep the processing of these         */
   /*            Callbacks small because other Events will not be able  */
   /*            to be called while one is being serviced.              */
typedef void (BTPSAPI *BTPM_Server_UnRegistration_Callback_t)(void *CallbackParameter);

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
   /* Bluetopia Platform Manager Services.  This function accepts as    */
   /* input, the Process ID (platform specific) of the client process   */
   /* that is calling this fuction, followed by a pointer to an         */
   /* initialization container structure that contains various platform */
   /* specific initialization information for various portions of the   */
   /* Platform Manager followed by an optional callback that can be     */
   /* installed to monitor the Server Registration.  If this parameter  */
   /* is specified, it will represent the caller defined callback that  */
   /* will be called when this module detects that the BTPM service has */
   /* exited.  This will allow the ability for the local module to clean*/
   /* up (i.e. call BTPM_Cleanup()).  The final parameter specifies the */
   /* caller defined Callback Parameter that will be passed to the      */
   /* installed callback function when it is dispatched.  This function */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Services.                                                         */
BTPSAPI_DECLARATION int BTPSAPI BTPM_Initialize(unsigned long ProcessID, BTPM_Initialization_Info_t *InitializationInfo, BTPM_Server_UnRegistration_Callback_t ServerUnRegistrationCallback, void *ServerUnRegistrationParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_Initialize_t)(unsigned long ProcessID, BTPM_Initialization_Info_t *InitializationInfo, BTPM_Server_UnRegistration_Callback_t ServerUnRegistrationCallback, void *ServerUnRegistrationParameter);
#endif

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Services.  After this function is      */
   /* called the Bluetooth Platform Manager service will no longer      */
   /* operate until it is initialized again via a call to the           */
   /* BTPM_Initialize() function.                                       */
BTPSAPI_DECLARATION void BTPSAPI BTPM_Cleanup(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BTPM_Cleanup_t)(void);
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
   /* debugging Module (for either the Local Library or Remote Service).*/
   /* This allows various levels of debug to be changed during run-time */
   /* to control debugging output.  This function accepts as input a    */
   /* flag that specifies whether or not the the Local Library (FALSE)  */
   /* or Remote Service (TRUE) is to have it's Debug Zone Mask updated. */
   /* The second parameter to this function specifies the new Debug Zone*/
   /* Mask to set for the Library or Service (specified by the first    */
   /* parameter).  This function returns zero if successful or a        */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI BTPM_SetDebugZoneMask(Boolean_t Remote, unsigned long DebugZoneMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_SetDebugZoneMask_t)(Boolean_t Remote, unsigned long DebugZoneMask);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to query the the current Debug Zone Mask that is being used*/
   /* by the debugging Module (either by the Local Library or the Remote*/
   /* Service).  This function accepts as it's first parameter, a flag  */
   /* that specifies whether or not to query the Debug Zone Mask of the */
   /* Local Library (FALSE) or Remote Service (TRUE).  The second       */
   /* Parameter is used to specify the Page Number of the Debug Zone    */
   /* Mask to query.  The final parameter specifies a pointer to a      */
   /* buffer that is to receive the Debug Zone Mask once it is queried. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * The second parameter must be less than the following     */
   /*          constant:                                                */
   /*                                                                   */
   /*             BTPM_DEBUG_ZONE_NUMBER_ZONE_PAGES                     */
   /*                                                                   */
BTPSAPI_DECLARATION int BTPSAPI BTPM_QueryDebugZoneMask(Boolean_t Remote, unsigned int PageNumber, unsigned long *DebugZoneMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_QueryDebugZoneMask_t)(Boolean_t Remote, unsigned int PageNumber, unsigned long *DebugZoneMask);
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
