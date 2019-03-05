/*****< fmpmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FMPMGR - FMP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/27/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMFMPM.h"            /* BTPM FMP Manager Prototypes/Constants.    */
#include "FMPMGR.h"              /* FMP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current IAS Instance ID.                 */
static unsigned int IASInstanceID;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _FMPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Internal Function Prototypes.                                     */
static void BTPSAPI IAS_Event_Callback(unsigned int BluetoothStackID, IAS_Event_Data_t *IAS_Event_Data, unsigned long CallbackParameter);

   /* The following function the function that is installed to process  */
   /* IAS Events from the stack.                                        */
static void BTPSAPI IAS_Event_Callback(unsigned int BluetoothStackID, IAS_Event_Data_t *IAS_Event_Data, unsigned long CallbackParameter)
{
   FMPM_Update_Data_t *FMPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((IAS_Event_Data) && (IAS_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no Event to dispatch.                       */
      FMPMUpdateData = NULL;

      switch(IAS_Event_Data->Event_Data_Type)
      {
         case etIAS_Server_Alert_Level_Control_Point_Command:
            /* Allocate memory to hold the Event Data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data) && (FMPMUpdateData = (FMPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(FMPM_Update_Data_t))) != NULL)
            {
               /* Note the Event type and copy the event data into the  */
               /* Notification structure.                               */
               FMPMUpdateData->UpdateType                                 = utFMPTargetEvent;
               FMPMUpdateData->UpdateData.TargetEventData.Event_Data_Type = IAS_Event_Data->Event_Data_Type;
               FMPMUpdateData->UpdateData.TargetEventData.Event_Data_Size = IAS_Event_Data->Event_Data_Size;

               BTPS_MemCopy(&(FMPMUpdateData->UpdateData.TargetEventData.Event_Data), IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data, IAS_Event_Data->Event_Data_Size);
            }
            break;
         default:
            /* Un-handled/unknown event.                                */
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(FMPMUpdateData)
      {
         if(!FMPM_NotifyUpdate(FMPMUpdateData))
            BTPS_FreeMemory((void *)FMPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the FMP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager FMP Manager  */
   /* Implementation.                                                   */
int _FMPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing FMP Manager (Imp)\n"));

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the FMP   */
   /* Manager Implementation.  After this function is called the FMP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _FMPM_Initialize() function.  */
void _FMPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Finally flag that this module is no longer initialized.        */
      Initialized  = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the FMP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the IAS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the FMP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _FMPM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   int          Result;
   unsigned int ServiceID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Initialize the Identifiers.                                 */
         IASInstanceID = 0;

         /* Stack has been powered up, so register an IAS Server        */
         /* Instance.                                                   */
         if(ServiceHandleRange)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to register IAS at 0x%04X - 0x%04X\n", ServiceHandleRange->Starting_Handle, ServiceHandleRange->Ending_Handle));

            /* Attempt to initialize the service at the specified handle*/
            /* range.                                                   */
            Result = IAS_Initialize_Service_Handle_Range(BluetoothStackID, IAS_Event_Callback, 0, &ServiceID, ServiceHandleRange);
         }
         else
            Result = IAS_Initialize_Service(BluetoothStackID, IAS_Event_Callback, 0, &ServiceID);

         if(Result > 0)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("IAS Service registered: GATT Service ID = %d\n", ServiceID));

            /* Save the FMP Instance ID.                                */
            IASInstanceID = (unsigned int)Result;

            /* FMP Framework initialized successfully.                  */
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("FMP Framework Initialized\n"));

            /* Save the Bluetooth Stack ID.                             */
            _BluetoothStackID = BluetoothStackID;
         }
         else
         {
            /* Error initializing FMP Framework.                        */
            DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_VERBOSE), ("FMP Service NOT registered: %d\n", Result));
         }

         /* Cleanup resources if an error occurred.                     */
         if(Result < 0)
         {
            if(IASInstanceID)
            {
               IAS_Cleanup_Service(_BluetoothStackID, IASInstanceID);

               IASInstanceID = 0;
            }
         }
      }
      else
      {
         /* Stack has been shutdown.                                    */
         IAS_Cleanup_Service(_BluetoothStackID, IASInstanceID);

         IASInstanceID     = 0;
         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the IAS Service registered by this    */
   /* module.                                                           */
unsigned int _FMPM_Query_Number_Attributes(void)
{
   unsigned int NumberOfAttributes;

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply call the Bluetopia function to query the number of         */
   /* attributes.                                                       */
   NumberOfAttributes = IAS_Query_Number_Attributes();

   DebugPrint((BTPM_DEBUG_ZONE_LE_FIND_ME | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", NumberOfAttributes));

   return(NumberOfAttributes);
}

