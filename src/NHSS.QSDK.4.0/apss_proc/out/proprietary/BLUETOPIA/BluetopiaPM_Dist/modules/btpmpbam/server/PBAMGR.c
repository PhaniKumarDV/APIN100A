/*****< pbamgr.c >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PBAMGR - Phone Book Access Manager Implementation for Stonestreet One     */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMPBAM.h"            /* BTPM PBAP Manager Prototypes/Constants.   */
#include "PBAMMSG.h"             /* BTPM PBAP Manager Message Formats.        */
#include "PBAMGR.h"              /* PBA Manager Impl. Prototypes/Constants.   */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _PBAM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Variable which indicates whether the PBAMClientInitializationData */
   /* member is valid.                                                  */
static Boolean_t ClientInitializationDataValid;

   /* Varaible which holds the client initialization info passed to the */
   /* PBAM Module.                                                      */
static PBAM_Client_Initialization_Data_t PBAMClientInitializationData;

   /* Variable which contains the SDP Service Record Handle returned    */
   /* when creating the PBAP-PCE service record.                        */
static DWord_t PCERecordHandle;

   /* Internal Function Prototypes.                                     */
static int MapPBAPServerErrorCode(int BluetopiaErrorCode);

static void BTPSAPI PBAP_Event_Callback(unsigned int BluetoothStackID, PBAP_Event_Data_t *PBAP_Event_Data, unsigned long CallbackParameter);

   /* This function is used to map Bluetopia return error codes to PM   */
   /* Error codes.                                                      */
static int MapPBAPServerErrorCode(int BluetopiaErrorCode)
{
   int ret_val;

   switch(BluetopiaErrorCode)
   {
      case BTPBAP_ERROR_NOT_INITIALIZED:
      case BTPBAP_ERROR_INVALID_BLUETOOTH_STACK_ID:
      case BTPBAP_ERROR_LIBRARY_INITIALIZATION_ERROR:
         ret_val = BTPM_ERROR_CODE_PLATFORM_MANAGER_NOT_INITIALIZED;
         break;
      case BTPBAP_ERROR_INVALID_PARAMETER:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
      case BTPBAP_ERROR_INSUFFICIENT_RESOURCES:
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         break;
      default:
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
         break;
   }

   return(ret_val);
}

   /* The following function the function that is installed to process  */
   /* Phone Book Access Events from the stack.                          */
static void BTPSAPI PBAP_Event_Callback(unsigned int BluetoothStackID, PBAP_Event_Data_t *PBAP_Event_Data, unsigned long CallbackParameter)
{
   int                 AdditionalSize;
   int                 AdditionalSize1;
   PBAM_Update_Data_t *PBAMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First verify that the input parameters appear to be semi-valid.   */
   if((PBAP_Event_Data) && (PBAP_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      PBAMUpdateData = NULL;

      switch(PBAP_Event_Data->Event_Data_Type)
      {
         case etPBAP_Open_Port_Confirmation:
         case etPBAP_Close_Port_Indication:
         case etPBAP_Abort_Confirmation:
         case etPBAP_Set_Phonebook_Confirmation:
         case etPBAP_Open_Port_Request_Indication:
         case etPBAP_Open_Port_Indication:
         case etPBAP_Abort_Indication:
            /* Allocate memory to hold the event data (we will process  */
            /* it later).                                               */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((PBAP_Event_Data->Event_Data.PBAP_Open_Port_Request_Indication_Data) && (PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the event data into the  */
               /* notification structure.                               */
               PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
               PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Open_Port_Request_Indication_Data, PBAP_Event_Data->Event_Data_Size);
            }
            break;
         case etPBAP_Pull_Phonebook_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->Buffer))
                  AdditionalSize = PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize;
               else
                  AdditionalSize  = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookConfirmationData.Buffer = ((unsigned char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookConfirmationData.Buffer, PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->Buffer, PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Confirmation_Data->BufferSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookConfirmationData.Buffer = NULL;
               }
            }
            break;
         case etPBAP_Pull_vCard_Listing_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->Buffer))
                  AdditionalSize = PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize;
               else
                  AdditionalSize  = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingConfirmationData.Buffer = ((unsigned char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingConfirmationData.Buffer, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->Buffer, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Confirmation_Data->BufferSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingConfirmationData.Buffer = NULL;
               }
            }
            break;
         case etPBAP_Pull_vCard_Entry_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->Buffer))
                  AdditionalSize = PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->BufferSize;
               else
                  AdditionalSize  = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardEntryConfirmationData.Buffer = ((unsigned char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardEntryConfirmationData.Buffer, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->Buffer, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Confirmation_Data->BufferSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardEntryConfirmationData.Buffer = NULL;
               }
            }
            break;
         case etPBAP_Pull_Phonebook_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Indication_Data->ObjectName))
                  AdditionalSize = BTPS_StringLength(PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Indication_Data->ObjectName) + 1;
               else
                  AdditionalSize  = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Indication_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookIndicationData.ObjectName = ((char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookIndicationData.ObjectName, PBAP_Event_Data->Event_Data.PBAP_Pull_Phonebook_Indication_Data->ObjectName, AdditionalSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookIndicationData.ObjectName = NULL;
               }
            }
            break;
         case etPBAP_Set_Phonebook_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Set_Phonebook_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Set_Phonebook_Indication_Data->ObjectName))
                  AdditionalSize = BTPS_StringLength(PBAP_Event_Data->Event_Data.PBAP_Set_Phonebook_Indication_Data->ObjectName) + 1;
               else
                  AdditionalSize  = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Set_Phonebook_Indication_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.SetPhonebookIndicationData.ObjectName = ((char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.SetPhonebookIndicationData.ObjectName, PBAP_Event_Data->Event_Data.PBAP_Set_Phonebook_Indication_Data->ObjectName, AdditionalSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.SetPhonebookIndicationData.ObjectName = NULL;
               }
            }
            break;
         case etPBAP_Pull_vCard_Listing_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data->ObjectName))
                  AdditionalSize = BTPS_StringLength(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data->ObjectName) + 1;
               else
                  AdditionalSize = 0;

               if((PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data->SearchValue))
                  AdditionalSize1 = BTPS_StringLength(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data->SearchValue) + 1;
               else
                  AdditionalSize1 = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize + AdditionalSize1)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingIndicationData.ObjectName = ((char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingIndicationData.ObjectName, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data->ObjectName, AdditionalSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullPhonebookIndicationData.ObjectName = NULL;

                  if(AdditionalSize1)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingIndicationData.SearchValue = ((char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t) + AdditionalSize;

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingIndicationData.SearchValue, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Listing_Indication_Data->SearchValue, AdditionalSize1);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardListingIndicationData.SearchValue = NULL;
               }
            }
            break;
         case etPBAP_Pull_vCard_Entry_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Indication_Data->ObjectName))
                  AdditionalSize = BTPS_StringLength(PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Indication_Data->ObjectName) + 1;
               else
                  AdditionalSize  = 0;

               if((PBAMUpdateData = (PBAM_Update_Data_t *)BTPS_AllocateMemory(sizeof(PBAM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  PBAMUpdateData->UpdateType                                    = utPhoneBookAccessEvent;
                  PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventType = PBAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData), PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Indication_Data, PBAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardEntryIndicationData.ObjectName = ((char *)PBAMUpdateData) + sizeof(PBAM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardEntryIndicationData.ObjectName, PBAP_Event_Data->Event_Data.PBAP_Pull_vCard_Entry_Indication_Data->ObjectName, AdditionalSize);
                  }
                  else
                     PBAMUpdateData->UpdateData.PhoneBookAccessEventData.EventData.PullvCardEntryIndicationData.ObjectName = NULL;
               }
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Un-handled/unknown event %d\n", PBAP_Event_Data->Event_Data_Type));
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(PBAMUpdateData)
      {
         if(!PBAM_NotifyUpdate(PBAMUpdateData))
            BTPS_FreeMemory((void *)PBAMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Phone Book Access Manager implementation. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Phone Book Access Manager implementation.                 */
int _PBAM_Initialize(PBAM_Initialization_Info_t *PBAMInitializationInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if this module has already been initialized.         */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Phone Book Access Manager (Imp)\n"));

      /* Check if any initialization data was supplied.                 */
      if(PBAMInitializationInfo)
      {
         /* Check if client information is supplied.                    */
         if((PBAMInitializationInfo->ClientInitializationData) && (PBAMInitializationInfo->ClientInitializationData->ServiceName))
         {
            /* Note the specified information (will be used to          */
            /* initialize the module when the device is powered On).    */
            PBAMClientInitializationData = *(PBAMInitializationInfo->ClientInitializationData);

            /* Note we have initialization data.                        */
            ClientInitializationDataValid = TRUE;
         }
         else
            ClientInitializationDataValid = FALSE;
      }
      else
         ClientInitializationDataValid = FALSE;

      /* Flag that this module is initialized.                          */
      Initialized = TRUE;

      ret_val     = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* Phone Book Access Manager implementation. After this function     */
   /* is called the Phone Book Access Manager implementation will no    */
   /* longer operate until it is initialized again via a call to the    */
   /* _PBAM_Initialize() function.                                      */
void _PBAM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Flag that this module is no longer initialized.                */
      Initialized = FALSE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Phone Book*/
   /* Access Manager implementation of that Bluetooth stack ID of the   */
   /* currently opened Bluetooth stack. When this parameter is set to   */
   /* non-zero, this function will actually initialize the Phone Book   */
   /* Access Manager with the specified Bluetooth stack ID. When this   */
   /* parameter is set to zero, this function will actually clean up all*/
   /* resources associated with the prior initialized Bluetooth Stack.  */
void _PBAM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   Byte_t       EIRData[2 + UUID_16_SIZE];
   DWord_t      Handle;
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if stack is being initialized OR it is being*/
      /* shutdown.                                                      */
      if(BluetoothStackID)
      {
         /* Phone Book Access Manager initialized successfully.         */
         DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Phone Book Access Manager Initialized\n"));

         _BluetoothStackID = BluetoothStackID;

         /* Check if the client initialization data is specified.       */
         if(ClientInitializationDataValid)
         {
            if(!PBAP_Register_Client_SDP_Record(_BluetoothStackID, PBAMClientInitializationData.ServiceName, &Handle))
            {
               /* Store the Client SDP Record Handle.                   */
               PCERecordHandle = Handle;

               /* Configure the EIR Data based on what is supported.    */
               EIRDataLength = (NON_ALIGNED_BYTE_SIZE + UUID_16_SIZE);

               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

               /* Assign the PBAP Client Role UUID (in big-endian       */
               /* format).                                              */
               SDP_ASSIGN_PBAP_CLIENT_UUID_16(tempUUID);

               /* Convert the UUID to little endian as required by EIR  */
               /* data.                                                 */
               CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), tempUUID);

               /* Increment the length we pass to the internal function */
               /* to take into account the length byte.                 */
               EIRDataLength += NON_ALIGNED_BYTE_SIZE;

               /* Configure the EIR data.                               */
               MOD_AddEIRData(EIRDataLength, EIRData);
            }
            else
               PCERecordHandle = 0;
         }
      }
      else
      {
         if(PCERecordHandle)
            SDP_Delete_Service_Record(_BluetoothStackID, PCERecordHandle);

         PCERecordHandle   = 0;

         _BluetoothStackID = 0;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Phone Book Access device.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function accepts the connection */
   /* information for the remote device (address and server port).      */
int _PBAM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort >= SPP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= SPP_PORT_NUMBER_MAXIMUM))
      {
         ret_val = PBAP_Open_Remote_Server_Port(_BluetoothStackID, RemoteDeviceAddress, RemoteServerPort, PBAP_Event_Callback, 0);

         /* If an error occurred, we need to map it to a valid error    */
         /* code.                                                       */
         if(ret_val <= 0)
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function exists to close an active Phone Book Access*/
   /* connection that was previously opened by a Successful call to     */
   /* _PBAM_Connect_Remote_Device() function.  This function accepts as */
   /* input the type of the local connection which should close its     */
   /* active connection.  This function returns zero if successful, or a*/
   /* negative return value if there was an error.                      */
int _PBAM_Disconnect_Device(unsigned int PBAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, Port ID: %d\n", PBAPID));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, check to see if the parameters appear to be semi-valid.  */
      if(PBAPID)
      {
         /* Go ahead and disconnect the device.                         */
         ret_val = PBAP_Close_Connection(_BluetoothStackID, PBAPID);

         /* If an error occurred we need to map it to the correct return*/
         /* value.                                                      */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_DISCONNECT_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding PBAP Profile Client Request.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Stack for which the PBAP  */
   /* Profile Client is valid.  The second parameter to this function   */
   /* specifies the PBAP ID (returned from a successful call to the     */
   /* _PBAM_Connect_Remote_Device() function).  This function returns   */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the PBAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a PBAP Profile Client Request that is to be aborted */
   /*          may have completed before the server was able to Abort   */
   /*          the request.  In either case, the caller will be notified*/
   /*          via PBAP Profile Callback of the status of the previous  */
   /*          Request.                                                 */
int _PBAM_Abort_Request(unsigned int PBAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (PBAPID))
   {
      if(!PBAP_Abort_Request(_BluetoothStackID, PBAPID))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function generates a PBAP Pull Phone Book request   */
   /* to the specified remote PBAP Server. The PBAPID parameter         */
   /* specifies the PBAP ID for the local PBAP Client (returned from a  */
   /* successful call to the _PBAM_ConnectRemoteDevice() function). The */
   /* PhoneBookNamePath parameter contains the name/path of the phone   */
   /* book being requested by this pull phone book operation. This value*/
   /* can be NULL if a phone book size request is being performed. The  */
   /* FilterLow parameter contains the lower 32 bits of the 64-bit      */
   /* filter attribute. The FilterHigh parameter contains the higher 32 */
   /* bits of the 64-bit filter attribute. The Format parameter is an   */
   /* enumeration which specifies the vCard format requested in this    */
   /* pull phone book request. If pfDefault is specified then the format*/
   /* will not be included in the request (note that the server will    */
   /* default to pfvCard21 in this case). The MaxListCount parameter    */
   /* is an unsigned integer that specifies the maximum number of       */
   /* entries the client can handle. A value of 65535 means that the    */
   /* number of entries is not restricted. A MaxListCount of ZERO (0)   */
   /* indicates that this is a request for the number of used indexes in*/
   /* the Phonebook specified by the PhoneBookNamePath parameter. The   */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this PullPhonebookRequest. This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP profile server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP profile server successfully */
   /*          executed the request.                                    */
   /* * NOTE * There can only be one outstanding PBAP profile request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          profile request cannot be issued until either the current*/
   /*          request is aborted (by calling the _PBAM_AbortRequest()  */
   /*          function) or the current request is completed (this is   */
   /*          signified by receiving a confirmation event in the PBAP  */
   /*          profile event callback that was registered when the PBAM */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_Phone_Book_Request(unsigned int PBAPID, char *PhoneBookNamePath, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t Format, Word_t MaxListCount, Word_t ListStartOffset)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (PBAPID))
   {
      if(!PBAP_Pull_Phonebook_Request(_BluetoothStackID, PBAPID, PhoneBookNamePath, FilterLow, FilterHigh, Format, MaxListCount, ListStartOffset))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function generates a PBAP Set Phonebook Request to  */
   /* the specified remote PBAP Server.  The BluetoothStackID parameter */
   /* the ID of the Bluetooth Stack that is associated with this PBAP   */
   /* Client.  The PBAPID parameter specifies the PBAP ID for the local */
   /* PBAP Client (returned from a successful call to the               */
   /* PBAP_Connect_Remote_Server_Port() function).  The PathOption      */
   /* parameter contains an enumerated value that indicates the type of */
   /* path change to request.  The ObjectName parameter contains the    */
   /* folder name to include with this Set Phonebook request.  This     */
   /* value can be NULL if no name is required for the selected         */
   /* PathOption.  See the PBAP specification for more information.     */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the PBAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Set_Phone_Book_Request(unsigned int PBAPID, PBAM_Set_Path_Option_t PathOption, char *ObjectName)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (PBAPID))
   {
      if(!PBAP_Set_Phonebook_Request(_BluetoothStackID, PBAPID, PathOption, ObjectName))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function generates a PBAP Pull vCard Listing        */
   /* Request to the specified remote PBAP Server. The PBAPID parameter */
   /* specifies the PBAP ID for the local PBAP Client (returned from a  */
   /* successful call to the _PBAM_Connect_Remote_Device() function).   */
   /* The ObjectName parameter contains the folder of the Phonebook     */
   /* being requested by this Pull vCard Listing operation. This value  */
   /* can be NULL if a PhonebookSize request is being performed. The    */
   /* ListOrder parameter is an enumerated type that determines the     */
   /* optionally requested order of the listing. Using the 'loDefault'  */
   /* value for this parameter will prevent this field from being added */
   /* to the request (note that the server will default to loIndexed    */
   /* in this case). The SearchAttribute is an enumerated type that     */
   /* determines the optionally requested attribute used to filter this */
   /* request. Using the 'saDefault' value for this parameter will      */
   /* prevent this field from being added to the request (note that the */
   /* server will default to saIndexed in this case). The SearchValue   */
   /* parameter contains an optional ASCII, Null-terminated character   */
   /* string that contains the string requested for search/filter. If   */
   /* this parameter is NULL, this field will be excluded from the      */
   /* request. The MaxListCount parameter is an unsigned integer that   */
   /* specifies the maximum number of list entries the client can       */
   /* handle. A value of 65535 means that the number of entries is not  */
   /* restricted. A MaxListCount of ZERO (0) indicates that this is a   */
   /* request for the number of used indexes in the Phonebook specified */
   /* by the ObjectName parameter. The ListStartOffset parameter        */
   /* specifies the index requested by the Client in this Pull vCard    */
   /* Listing. This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the PBAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_vCard_Listing_Request(unsigned int PBAPID, char *ObjectName, PBAM_List_Order_t ListOrder, PBAM_Search_Attribute_t SearchAttribute, char *SearchValue, Word_t MaxListCount, Word_t ListStartOffset)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (PBAPID))
   {
      if(!PBAP_Pull_vCard_Listing_Request(_BluetoothStackID, PBAPID, ObjectName, ListOrder, SearchAttribute, SearchValue, MaxListCount, ListStartOffset))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function generates a PBAP Pull vCard Entry Request  */
   /* to the specified remote PBAP Server. The PBAPID parameter         */
   /* specifies the PBAP ID for the local PBAP Client (returned from a  */
   /* successful call to the _PBAM_Connect_Remote_Device() function).   */
   /* The ObjectName parameter contains the name of the Phonebook       */
   /* entry being requested by this Pull vCard Entry operation. The     */
   /* FilterLow parameter contains the lower 32 bits of the 64-bit      */
   /* filter attribute. The FilterHigh parameter contains the higher 32 */
   /* bits of the 64-bit filter attribute. The Format parameter is an   */
   /* enumeration which specifies the vCard format requested in this    */
   /* Pull vCard Entry request. If pfDefault is specified then the      */
   /* format will not be included in the request (note that in this case*/
   /* the server will default to pfvCard21 in this case). This function */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          PBAP Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote PBAP Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding PBAP Profile Request   */
   /*          active at any one time.  Because of this, another PBAP   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the PBAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the PBAP  */
   /*          Profile Event Callback that was registered when the PBAP */
   /*          Profile Port was opened).                                */
int _PBAM_Pull_vCard_Entry_Request(unsigned int PBAPID, char *ObjectName, DWord_t FilterLow, DWord_t FilterHigh, PBAM_VCard_Format_t Format)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if((Initialized) && (PBAPID))
   {
      if(!PBAP_Pull_vCard_Entry_Request(_BluetoothStackID, PBAPID, ObjectName, FilterLow, FilterHigh, Format))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for opening a local PBAP    */
   /* Server.  The ServerPort parameter is the Port on which to open    */
   /* this server, and *MUST* be between PBAP_PORT_NUMBER_MINIMUM and   */
   /* PBAP_PORT_NUMBER_MAXIMUM.  The SupportedRepositories parameter    */
   /* is a bitmask which determines which repositories are supported    */
   /* by this server instance.  This function returns a positive, non   */
   /* zero value if successful or a negative return error code if an    */
   /* error occurs.  A successful return code will be a PBAP Profile ID */
   /* that can be used to reference the Opened PBAP Profile Server Port */
   /* in ALL other PBAP Server functions in this module.  Once an PBAP  */
   /* Profile Server is opened, it can only be Un-Registered via a call */
   /* to the PBAP_Close_Server() function (passing the return value from*/
   /* this function).                                                   */
int _PBAM_Open_Server(unsigned int ServerPort, Byte_t SupportedRepositories)
{
   int Result;
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((Result = PBAP_Open_Server_Port(_BluetoothStackID, ServerPort, SupportedRepositories, PBAP_Event_Callback, 0)) > 0)
      {
         ret_val = Result;

         if((Result = PBAP_Set_Server_Mode(_BluetoothStackID, (unsigned int)ret_val, PBAP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION)) != 0)
         {
            PBAP_Close_Server_Port(_BluetoothStackID, (unsigned int)ret_val);

            ret_val = MapPBAPServerErrorCode(Result);
         }
      }
      else
         ret_val = MapPBAPServerErrorCode(Result);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for closing a PBAP          */
   /* Profile Server (which was opened by a successful call to the      */
   /* PBAM_Register_Server() function).  The first parameter is the     */
   /* Bluetooth Stack ID of the previously opened server port.  The     */
   /* second parameter is the PBAP ID returned from the previous call to*/
   /* PBAM_Register_Server().  This function returns zero if successful,*/
   /* or a negative return error code if an error occurred.  Note that  */
   /* this function does NOT delete any SDP Service Record Handles.     */
int _PBAM_Close_Server(unsigned int PBAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Close_Server_Port(_BluetoothStackID, PBAPID)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function adds a PBAP Server (PSE) Service Record    */
   /* to the SDP Database.  The PBAPID parameter is the PBAP ID that    */
   /* was returned by a previous call to PBAP_Open_Server_Port().  The  */
   /* ServiceName parameter is a pointer to ASCII, NULL terminated      */
   /* string containing the Service Name to include within the SDP      */
   /* Record.  The ServiceRecordHandle parameter is a pointer to a      */
   /* DWord_t which receives the SDP Service Record Handle if this      */
   /* function successfully creates an SDP Service Record.  If this     */
   /* function returns zero, then the SDPServiceRecordHandle entry will */
   /* contain the Service Record Handle of the added SDP Service Record.*/
   /* If this function fails, a negative return error code will be      */
   /* returned and the SDPServiceRecordHandle value will be undefined.  */
int _PBAM_Register_Service_Record(unsigned int PBAPID, char *ServiceName, DWord_t *ServiceRecordHandle)
{
   int          ret_val;
   Byte_t       EIRData[2 + UUID_16_SIZE];
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Register_Server_SDP_Record(_BluetoothStackID, PBAPID, ServiceName, ServiceRecordHandle)) == 0)
      {
         /* Configure the EIR Data based on what is supported.          */
         EIRDataLength = (NON_ALIGNED_BYTE_SIZE + UUID_16_SIZE);

         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

         /* Assign the PBAP Server Role UUID (in big-endian format).    */
         SDP_ASSIGN_PBAP_SERVER_UUID_16(tempUUID);

         /* Convert the UUID to little endian as required by EIR data.  */
         CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), tempUUID);

         /* Increment the length we pass to the internal function to    */
         /* take into account the length byte.                          */
         EIRDataLength += NON_ALIGNED_BYTE_SIZE;

         /* Configure the EIR data.                                     */
         MOD_AddEIRData(EIRDataLength, EIRData);
      }
      else
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* This function removes a PBAP Server (PSE) Service Record from     */
   /* the SDP database. The PBAPID parameter is the PBAP ID that was    */
   /* returned by a previous call to PBAM_Register_Server(). The        */
   /* ServiceRecordHandle is the handle of the PBAP service record to   */
   /* remove. This function returns zero if successful and a negative   */
   /* return error code if there is an error.                           */
int _PBAM_Un_Register_Service_Record(unsigned int PBAPID, DWord_t ServiceRecordHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Un_Register_SDP_Record(_BluetoothStackID, PBAPID, ServiceRecordHandle)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is provided to allow a means to respond    */
   /* to a request to connect to the local PBAP Profile Server.  The    */
   /* first parameter is the PBAP ID that was returned from a previous  */
   /* PBAP_Open_Server_Port() function for this server.  The final      */
   /* parameter to this function is a Boolean_t that indicates whether  */
   /* to accept the pending connection.  This function returns zero if  */
   /* successful, or a negative return value if there was an error.     */
int _PBAM_Open_Request_Response(unsigned int PBAPID, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Open_Request_Response(_BluetoothStackID, PBAPID, Accept)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function sends a PBAP Pull Phonebook Response to the*/
   /* specified remote PBAP Client.  This is used for responding to a   */
   /* PBAP Pull Phonebook Indication.  The PBAPID parameter specifies   */
   /* the PBAP ID of the PBAP Server responding to the request.  The    */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  The PhonebookSize parameter is a pointer to a variable */
   /* that can optionally contain a Phonebook Size value to return in   */
   /* this request.  This should be done if the received indication     */
   /* indicated a request for PhonebookSize by indicating a MaxListCount*/
   /* = ZERO (0).  If this value is to be included in the response the  */
   /* Buffer parameter should be set to NULL and the BufferSize to ZERO.*/
   /* If this value is NOT to be used in the response, this parameter   */
   /* should be set to NULL.  The NewMissedCalls parameter is a pointer */
   /* to a variable that can optionally contain the number of new missed*/
   /* calls which have not been checked on this server.  This should    */
   /* only be included on requests for the 'mch' phonebook type.  If    */
   /* this value is to be included in the response the Buffer parameter */
   /* should be set to NULL and the BufferSize to ZERO.  If this value  */
   /* is NOT to be used in the response, this parameter should be set   */
   /* to NULL.  The BufferSize parameter is the size in bytes of the    */
   /* data included in the specified Buffer.  The Buffer parameter is   */
   /* a pointer to a byte buffer containing the Phonebook data to be    */
   /* included in this response packet.  The AmountWritten parameter is */
   /* a pointer to variable which will be written with the actual amount*/
   /* of data that was able to be included in the packet.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PBAM_Pull_Phonebook_Response(unsigned int PBAPID, Byte_t ResponseCode, Word_t *PhonebookSize, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Pull_Phonebook_Response(_BluetoothStackID, PBAPID, ResponseCode, PhonebookSize, NewMissedCalls, BufferSize, Buffer, AmountWritten)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function sends a PBAP Set Phonebook Response to the */
   /* specified remote PBAP Client.  This is used for responding to a   */
   /* PBAP Set Phonebook Indication.  The PBAPID parameter specifies    */
   /* the PBAP ID of the PBAP Server responding to the request.  The    */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  This function returns zero if successful or a negative */
   /* return error code if there was an error.                          */
int _PBAM_Set_Phonebook_Response(unsigned int PBAPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Set_Phonebook_Response(_BluetoothStackID, PBAPID, ResponseCode)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function sends a PBAP Pull vCard Listing Response   */
   /* to the specified remote PBAP Client.  This is used for responding */
   /* to a PBAP Pull vCard Listing Indication.  The PBAPID parameter    */
   /* specifies the PBAP ID of the PBAP Server responding to the        */
   /* request.  The ResponseCode parameter is the OBEX response code    */
   /* to include in the response.  The PhonebookSize parameter is a     */
   /* pointer to a variable that can optionally contain a Phonebook     */
   /* Size value to return in this request.  This should be done if     */
   /* the received indication indicated a request for PhonebookSize     */
   /* by indicating a MaxListCount = ZERO (0).  If this value is to     */
   /* be included in the response the Buffer parameter should be set    */
   /* to NULL and the BufferSize to ZERO.  If this value is NOT to be   */
   /* used in the response, this parameter should be set to NULL.  The  */
   /* NewMissedCalls parameter is a pointer to a variable that can      */
   /* optionally contain the number of new missed calls which have      */
   /* not been checked on this server.  This should only be included    */
   /* on requests for the 'mch' phonebook type.  If this value is to    */
   /* be included in the response the Buffer parameter should be set    */
   /* to NULL and the BufferSize to ZERO.  If this value is NOT to be   */
   /* used in the response, this parameter should be set to NULL.  The  */
   /* BufferSize parameter is the size in bytes of the data included    */
   /* in the specified Buffer.  The Buffer parameter is a pointer to a  */
   /* byte buffer containing the Phonebook listing data to be included  */
   /* in this response packet.  The AmountWritten parameter is a pointer*/
   /* to variable which will be written with the actual amount of data  */
   /* that was able to be included in the packet.  This function returns*/
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _PBAM_Pull_vCard_Listing_Response(unsigned int PBAPID, Byte_t ResponseCode, Word_t *PhonebookSize, Byte_t *NewMissedCalls, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Pull_vCard_Listing_Response(_BluetoothStackID, PBAPID, ResponseCode, PhonebookSize, NewMissedCalls, BufferSize, Buffer, AmountWritten)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function sends a PBAP Pull vCard Entry Response to  */
   /* the specified remote PBAP Client.  This is used for responding    */
   /* to a PBAP Pull vCard Entry Indication.  The PBAPID parameter      */
   /* specifies the PBAP ID of the PBAP Server responding to the        */
   /* request.  The ResponseCode parameter is the OBEX response code    */
   /* to include in the response.  The BufferSize parameter is the      */
   /* size in bytes of the data included in the specified Buffer.  The  */
   /* Buffer parameter is a pointer to a byte buffer containing the     */
   /* Phonebook entry data to be included in this response packet.      */
   /* The AmountWritten parameter is a pointer to variable which will   */
   /* be written with the actual amount of data that was able to be     */
   /* included in the packet.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
int _PBAM_Pull_vCard_Entry_Response(unsigned int PBAPID, Byte_t ResponseCode, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to see if this module has been previously            */
   /* initialized.                                                      */
   if(Initialized)
   {
      if((ret_val = PBAP_Pull_vCard_Entry_Response(_BluetoothStackID, PBAPID, ResponseCode, BufferSize, Buffer, AmountWritten)) != 0)
         ret_val = MapPBAPServerErrorCode(ret_val);
   }
   else
      ret_val = BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

