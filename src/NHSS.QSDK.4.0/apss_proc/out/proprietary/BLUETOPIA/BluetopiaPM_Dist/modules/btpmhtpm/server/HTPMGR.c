/*****< htpmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMGR - Health Thermometer Manager Implementation for Stonestreet One    */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMHTPM.h"            /* BTPM HTP Manager Prototypes/Constants.    */
#include "HTPMGR.h"              /* HTP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HTP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HTP Manager  */
   /* Implementation.                                                   */
int _HTPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HTP Manager (Imp)\n"));

      /* Note the specified information (will be used to initialize the */
      /* module when the device is powered On).                         */

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the HTP   */
   /* Manager Implementation.  After this function is called the HTP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HTPM_Initialize() function.  */
void _HTPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the HTP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HTP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HTPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Do Nothing.                                                    */
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

