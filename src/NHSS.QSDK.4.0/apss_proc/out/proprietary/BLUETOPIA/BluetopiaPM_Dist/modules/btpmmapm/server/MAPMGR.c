/*****< mapmgr.c >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAPMGR - Message Access Manager Implementation for Stonestreet One        */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/03/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMMAPM.h"            /* BTPM MAP Manager Prototypes/Constants.    */
#include "MAPMMSG.h"             /* BTPM MAP Manager Message Formats.         */
#include "MAPMGR.h"              /* MAP Manager Impl. Prototypes/Constants.   */
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
   /* _MAPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Internal Function Prototypes.                                     */
static unsigned int CalculateFolderNameSize(Word_t *FolderName);

static void BTPSAPI MAP_Event_Callback(unsigned int BluetoothStackID, MAP_Event_Data_t *MAP_Event_Data, unsigned long CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* determine the size (in bytes) required to hold the specified      */
   /* Folder Name (including terminating NULL character).               */
static unsigned int CalculateFolderNameSize(Word_t *FolderName)
{
   unsigned int ret_val;

   ret_val = 0;

   if(FolderName)
   {
      while(*(FolderName++))
         ret_val += NON_ALIGNED_WORD_SIZE;

      /* Make sure we count the NULL terminator.                        */
      ret_val += NON_ALIGNED_WORD_SIZE;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* MAP_Event_Callback processes bluetopia message access events.     */
static void BTPSAPI MAP_Event_Callback(unsigned int BluetoothStackID, MAP_Event_Data_t *MAP_Event_Data, unsigned long CallbackParameter)
{
   unsigned int        AdditionalSize;
   MAPM_Update_Data_t *MAPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the event data pointer is not null and contains data. */
   if((MAP_Event_Data) && (MAP_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      MAPMUpdateData = NULL;

      switch(MAP_Event_Data->Event_Data_Type)
      {
         case etMAP_Open_Request_Indication:
         case etMAP_Open_Port_Indication:
         case etMAP_Open_Port_Confirmation:
         case etMAP_Close_Port_Indication:
         case etMAP_Send_Event_Confirmation:
         case etMAP_Notification_Registration_Indication:
         case etMAP_Notification_Registration_Confirmation:
         case etMAP_Get_Folder_Listing_Indication:
         case etMAP_Set_Message_Status_Confirmation:
         case etMAP_Update_Inbox_Indication:
         case etMAP_Update_Inbox_Confirmation:
         case etMAP_Set_Folder_Confirmation:
         case etMAP_Abort_Confirmation:
         case etMAP_Abort_Indication:

            /* Allocate memory and copy the Bluetopia Event data to pass*/
            /* on to the PM client.                                     */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((MAP_Event_Data->Event_Data.MAP_Open_Request_Indication_Data) && (MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the event data into the  */
               /* notification structure.                               */
               MAPMUpdateData->UpdateType                        = utMAPEvent;
               MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Open_Request_Indication_Data, MAP_Event_Data->Event_Data_Size);
            }
            break;
         case etMAP_Send_Event_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Send_Event_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Send_Event_Indication_Data->DataBuffer))
                  AdditionalSize = MAP_Event_Data->Event_Data.MAP_Send_Event_Indication_Data->DataLength;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Send_Event_Indication_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.SendEventIndicationData.DataBuffer = ((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.SendEventIndicationData.DataBuffer, MAP_Event_Data->Event_Data.MAP_Send_Event_Indication_Data->DataBuffer, AdditionalSize);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", AdditionalSize));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.SendEventIndicationData.DataBuffer = NULL;
               }
            }
            break;
         case etMAP_Get_Folder_Listing_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataBuffer))
                  AdditionalSize = MAP_Event_Data->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataLength;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetFolderListingConfirmationData.DataBuffer = ((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetFolderListingConfirmationData.DataBuffer, MAP_Event_Data->Event_Data.MAP_Get_Folder_Listing_Confirmation_Data->DataBuffer, AdditionalSize);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", AdditionalSize));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetFolderListingConfirmationData.DataBuffer = NULL;
               }
            }
            break;
         case etMAP_Get_Message_Listing_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               AdditionalSize = 0;

               /* Determine the length of the Folder Name.              */
               if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->FolderName)
                  AdditionalSize = CalculateFolderNameSize(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->FolderName);

               if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterRecipient)
                  AdditionalSize += BTPS_StringLength(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterRecipient) + 1;

               if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterOriginator)
                  AdditionalSize += BTPS_StringLength(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterOriginator) + 1;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     /* Reset the AdditionalSize variable (we will use  */
                     /* this to be an offset of where we need to copy   */
                     /* data).                                          */
                     AdditionalSize = 0;

                     if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->FolderName)
                     {
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.FolderName = (Word_t *)(((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t));

                        /* Note the Folder Name Size (in bytes).        */
                        AdditionalSize = CalculateFolderNameSize(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->FolderName);

                        /* Now copy the buffer.                         */
                        BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.FolderName, MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->FolderName, AdditionalSize);

                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("FolderName Length %d\n", AdditionalSize));
                     }
                     else
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.FolderName = NULL;

                     if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterRecipient)
                     {
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterRecipient = ((char *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t) + AdditionalSize;

                        /* Now copy the buffer.                         */
                        BTPS_StringCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterRecipient, MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterRecipient);

                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("FilterRecipient: %s\n", MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterRecipient));

                        AdditionalSize += BTPS_StringLength(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterRecipient);
                     }
                     else
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterRecipient = NULL;

                     if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterOriginator)
                     {
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterOriginator = ((char *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t) + AdditionalSize;

                        /* Now copy the buffer.                         */
                        BTPS_StringCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterOriginator, MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Indication_Data->ListingInfo.FilterRecipient);

                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("FilterOriginator: %s\n", MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterOriginator));
                     }
                     else
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingIndicationData.ListingInfo.FilterOriginator = NULL;
                  }
               }
            }
            break;
         case etMAP_Get_Message_Listing_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataBuffer))
                  AdditionalSize = MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataLength;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Confirmation_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingConfirmationData.DataBuffer = ((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingConfirmationData.DataBuffer, MAP_Event_Data->Event_Data.MAP_Get_Message_Listing_Confirmation_Data->DataBuffer, AdditionalSize);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", AdditionalSize));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageListingConfirmationData.DataBuffer = NULL;
               }
            }
            break;
         case etMAP_Get_Message_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Get_Message_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Get_Message_Indication_Data->MessageHandle))
                  AdditionalSize = BTPS_StringLength(MAP_Event_Data->Event_Data.MAP_Get_Message_Indication_Data->MessageHandle) + 1;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Get_Message_Indication_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageIndicationData.MessageHandle = ((char *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_StringCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageIndicationData.MessageHandle, MAP_Event_Data->Event_Data.MAP_Get_Message_Indication_Data->MessageHandle);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MessageHandle: %s\n", MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageIndicationData.MessageHandle));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageIndicationData.MessageHandle = NULL;
               }
            }
            break;
         case etMAP_Get_Message_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Get_Message_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Get_Message_Confirmation_Data->DataBuffer))
                  AdditionalSize = MAP_Event_Data->Event_Data.MAP_Get_Message_Confirmation_Data->DataLength;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Get_Message_Confirmation_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageConfirmationData.DataBuffer = ((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageConfirmationData.DataBuffer, MAP_Event_Data->Event_Data.MAP_Get_Message_Confirmation_Data->DataBuffer, AdditionalSize);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", AdditionalSize));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.GetMessageConfirmationData.DataBuffer = NULL;
               }
            }
            break;
         case etMAP_Set_Message_Status_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Set_Message_Status_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Set_Message_Status_Indication_Data->MessageHandle))
                  AdditionalSize = BTPS_StringLength(MAP_Event_Data->Event_Data.MAP_Set_Message_Status_Indication_Data->MessageHandle) + 1;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Set_Message_Status_Indication_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.SetMessageStatusIndicationData.MessageHandle = ((char *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_StringCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.SetMessageStatusIndicationData.MessageHandle, MAP_Event_Data->Event_Data.MAP_Set_Message_Status_Indication_Data->MessageHandle);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MessageHandle: %s\n", MAPMUpdateData->UpdateData.MAPEventData.EventData.SetMessageStatusIndicationData.MessageHandle));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.SetMessageStatusIndicationData.MessageHandle = NULL;
               }
            }
            break;
         case etMAP_Push_Message_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               AdditionalSize = 0;

               if((MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->FolderName))
                  AdditionalSize = CalculateFolderNameSize(MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->FolderName);

               if((MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->DataBuffer))
                  AdditionalSize += MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->DataLength;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     /* Reset the AdditionalSize variable (we will use  */
                     /* this to be an offset of where we need to copy   */
                     /* data).                                          */
                     AdditionalSize = 0;

                     if(MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->FolderName)
                     {
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.FolderName = (Word_t *)(((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t));

                        /* Note the Folder Name Size (in bytes).        */
                        AdditionalSize = CalculateFolderNameSize(MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->FolderName);

                        /* Now copy the buffer.                         */
                        BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.FolderName, MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->FolderName, AdditionalSize);

                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("FolderName Length: %d\n", AdditionalSize));
                     }
                     else
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.FolderName = NULL;

                     if((MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->DataBuffer) && (MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->DataLength))
                     {
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.DataBuffer = ((Byte_t *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t) + AdditionalSize;

                        /* Now copy the buffer.                         */
                        BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.DataBuffer, MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->DataBuffer, MAP_Event_Data->Event_Data.MAP_Push_Message_Indication_Data->DataLength);

                        DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.DataLength));
                     }
                     else
                        MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageIndicationData.DataBuffer = NULL;
                  }
               }
            }
            break;
         case etMAP_Push_Message_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Push_Message_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Push_Message_Confirmation_Data->MessageHandle))
                  AdditionalSize = BTPS_StringLength(MAP_Event_Data->Event_Data.MAP_Push_Message_Confirmation_Data->MessageHandle) + 1;
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Push_Message_Confirmation_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageConfirmationData.MessageHandle = ((char *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_StringCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageConfirmationData.MessageHandle, MAP_Event_Data->Event_Data.MAP_Push_Message_Confirmation_Data->MessageHandle);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("MessageHandle: %s\n", MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageConfirmationData.MessageHandle));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.PushMessageConfirmationData.MessageHandle = NULL;
               }
            }
            break;
         case etMAP_Set_Folder_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(MAP_Event_Data->Event_Data.MAP_Set_Folder_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((MAP_Event_Data->Event_Data.MAP_Set_Folder_Indication_Data->FolderName))
                  AdditionalSize = CalculateFolderNameSize(MAP_Event_Data->Event_Data.MAP_Set_Folder_Indication_Data->FolderName);
               else
                  AdditionalSize  = 0;

               if((MAPMUpdateData = (MAPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(MAPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  MAPMUpdateData->UpdateType                        = utMAPEvent;
                  MAPMUpdateData->UpdateData.MAPEventData.EventType = MAP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(MAPMUpdateData->UpdateData.MAPEventData.EventData), MAP_Event_Data->Event_Data.MAP_Set_Folder_Indication_Data, MAP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.SetFolderIndicationData.FolderName = (Word_t *)(((char *)MAPMUpdateData) + sizeof(MAPM_Update_Data_t));

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(MAPMUpdateData->UpdateData.MAPEventData.EventData.SetFolderIndicationData.FolderName, MAP_Event_Data->Event_Data.MAP_Set_Folder_Indication_Data->FolderName, AdditionalSize);

                     DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("FolderName Length: %d\n", AdditionalSize));
                  }
                  else
                     MAPMUpdateData->UpdateData.MAPEventData.EventData.SetFolderIndicationData.FolderName = NULL;
               }
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_WARNING), ("Unhandled Event %d\n", MAP_Event_Data->Event_Data_Type));
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(MAPMUpdateData)
      {
         if(!MAPM_NotifyUpdate(MAPMUpdateData))
            BTPS_FreeMemory((void *)MAPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Bluetopia Platform Manager Server Manager Message Access Functions*/
   /* The PM server manager functions provide an interface to bluetopia */
   /* functions.                                                        */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Message Access Manager implementation.  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Message Access Manager implementation.                    */
int _MAPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has already been initialized.          */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Message Access Manager \n"));

      /* Flag that the module is initialized.                           */
      Initialized       = TRUE;

      /* Flag that that Bluetooth Stack is not currently open.          */
      _BluetoothStackID = 0;

      /* Flag success to the caller.                                    */
      ret_val           = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for shutting down the       */
   /* Message Access Manager implementation.  After this function is    */
   /* called the Message Access Manager implementation will no longer   */
   /* operate until it is initialized again via a call to the           */
   /* _MAPM_Initialize() function.                                      */
void _MAPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Flag that this module is no longer initialized.                */
      Initialized       = FALSE;

      /* Flag that the Stack is not open.                               */
      _BluetoothStackID = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Message   */
   /* Access Manager implementation of that Bluetooth stack ID of the   */
   /* currently opened Bluetooth stack.  When this parameter is set to  */
   /* non-zero, this function will actually initialize the Message      */
   /* Access Manager with the specified Bluetooth stack ID.  When this  */
   /* parameter is set to zero, this function will actually clean up all*/
   /* resources associated with the prior initialized Bluetooth Stack.  */
void _MAPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Note the Bluetooth Stack ID (regardless if it's zero.          */
      _BluetoothStackID = BluetoothStackID;
   }

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for opening a local MAP     */
   /* Server.  The parameter to this function is the Port on which to   */
   /* open this server, and *MUST* be between MAP_PORT_NUMBER_MINIMUM   */
   /* and MAP_PORT_NUMBER_MAXIMUM.  This function returns a positive,   */
   /* non zero value if successful or a negative return error code if an*/
   /* error occurs.  A successful return code will be a MAP Profile ID  */
   /* that can be used to reference the Opened MAP Profile Server Port  */
   /* in ALL other MAP Server functions in this module.  Once an MAP    */
   /* Profile Server is opened, it can only be Un-Registered via a call */
   /* to the _MAP_Close_Server() function (passing the return value from*/
   /* this function).                                                   */
int _MAP_Open_Message_Access_Server(unsigned int MessageAccessServiceServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the port is in the allowed range.              */
      if((MessageAccessServiceServerPort >= MAP_PORT_NUMBER_MINIMUM) && (MessageAccessServiceServerPort <= MAP_PORT_NUMBER_MAXIMUM))
      {
         ret_val = MAP_Open_Message_Access_Server(_BluetoothStackID, MessageAccessServiceServerPort, MAP_Event_Callback, 0);

//xxx This is not the right error code. We need new errors.

         /* If no error occurred, set the server mode to manual accept. */
         if(ret_val > 0)
         {
            if(MAP_Set_Server_Mode(_BluetoothStackID, ret_val, MAP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION))
            {
               /* An error occurred, so go ahead and close the server.  */
               MAP_Close_Server(_BluetoothStackID, ret_val);

               /* Flag an error.                                        */
               ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for opening a local MAP     */
   /* NotificationServer.  The first parameter to this function is the  */
   /* port to use to open this server, and *MUST* be between            */
   /* MAP_PORT_NUMBER_MINIMUM and MAP_PORT_NUMBER_MAXIMUM.  The second  */
   /* parameter is the MAPID for the Message Access Client that the     */
   /* server is being associated with.  This function returns a         */
   /* positive, non zero value if successful or a negative return error */
   /* code if an error occurs.  A successful return code will be a MAP  */
   /* Profile ID that can be used to reference the Opened MAP Profile   */
   /* Server Port in ALL other MAP Server functions in this module.     */
   /* Once an MAP Profile Server is opened, it can only be Un-Registered*/
   /* via a call to the _MAP_Close_Server() function (passing the return*/
   /* value from this function).                                        */
int _MAP_Open_Message_Notification_Server(unsigned int MessageNotificationServiceServerPort, unsigned int MAS_MAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the port is in the allowed range.              */
      if((MessageNotificationServiceServerPort >= MAP_PORT_NUMBER_MINIMUM) && (MessageNotificationServiceServerPort <= MAP_PORT_NUMBER_MAXIMUM))
      {
         ret_val = MAP_Open_Message_Notification_Server(_BluetoothStackID, MessageNotificationServiceServerPort, MAS_MAPID, MAP_Event_Callback, 0);

         /* If an error occurred, map it to a valid error code          */
         if(ret_val <= 0)
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Server.  The first       */
   /* parameter to this function is the MAP ID of the Server for which a*/
   /* connection request was received.  The final parameter to this     */
   /* function specifies whether to accept the pending connection       */
   /* request (or to reject the request).  This function returns zero if*/
   /* successful, or a negative return error code if an error occurred. */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etMAP_Open_Service_Port_Indication event has occurred. */
int _MAP_Open_Request_Response(unsigned int MAPID, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Open_Request_Response(_BluetoothStackID, MAPID, AcceptConnection))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function adds a MAP Server (MSE) Service Record to  */
   /* the SDP Database.  The first parameter is the MAP ID that was     */
   /* returned by a previous call to _MAP_Open_Message_Access_Server(). */
   /* The second parameter is a pointer to ASCII, NULL terminated string*/
   /* containing the Service Name to include within the SDP Record.  The*/
   /* third parameter is a user defined MAS Instance value.  The fourth */
   /* parameter is the Supported Message Type Bitmask value.  This      */
   /* function returns a positive, non-zero value on success or a       */
   /* negative return error code if there was an error.  If this        */
   /* function returns success then the return value is the SDP Service */
   /* Record Handle of the Record that was added.                       */
   /* * NOTE * This function should only be called with the MAP ID that */
   /*          was returned from the _MAP_Open_Message_Access_Server()  */
   /*          function.  This function should NEVER be used with MAP ID*/
   /*          returned from the                                        */
   /*          _MAP_Open_Remote_Message_Access_Server_Port() function.  */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the _MAP_Un_Register_SDP_Record()  */
   /*          function.                                                */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
long _MAP_Register_Message_Access_Server_SDP_Record(unsigned int MAPID, char *ServiceName, Byte_t MASInstance, Byte_t SupportedMessageTypes)
{
   long         ret_val;
   Byte_t       EIRData[2 + UUID_16_SIZE];
   DWord_t      SDPServiceRecordHandle;
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Register_Message_Access_Server_SDP_Record(_BluetoothStackID, MAPID, ServiceName, MASInstance, SupportedMessageTypes, &SDPServiceRecordHandle))
      {
         /* Return the SDP Record Handle.                               */
         ret_val = (long)SDPServiceRecordHandle;

         /* Configure the EIR Data.                                     */
         EIRDataLength = NON_ALIGNED_BYTE_SIZE + UUID_16_SIZE;

         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

         /* Assign the MAS Role UUID (in big-endian format).            */
         SDP_ASSIGN_MESSAGE_ACCESS_SERVER_UUID_16(tempUUID);

         /* Convert the UUID to little endian as required by EIR data.  */
         CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), tempUUID);

         /* Increment the length we pass to the internal function to    */
         /* take into account the length byte.                          */
         EIRDataLength += NON_ALIGNED_BYTE_SIZE;

         /* Configure the EIR data.                                     */
         MOD_AddEIRData(EIRDataLength, EIRData);
      }
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %ld\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function adds a MAP Server (MSE) Service Record to  */
   /* the SDP Database.  The first parameter to this function is the MAP*/
   /* ID that was returned by a previous call to                        */
   /* _MAP_Open_Message_Access_Server().  The second parameter is a     */
   /* pointer to ASCII, NULL terminated string containing the Service   */
   /* Name to include within the SDP Record.  The third parameter is a  */
   /* user defined MAS Instance value.  The fourth parameter is the     */
   /* Supported Message Type Bitmask value.  This function returns a    */
   /* positive, non-zero value on success or a negative return error    */
   /* code if there was an error.  If this function returns success then*/
   /* the return value is the SDP Service Record Handle of the Record   */
   /* that was added.                                                   */
   /* * NOTE * This function should only be called with the MAP ID that */
   /*          was returned from the _MAP_Open_Message_Access_Server()  */
   /*          function.  This function should NEVER be used with MAP ID*/
   /*          returned from the                                        */
   /*          _MAP_Open_Remote_Message_Notification_Server_Port()      */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the MAP_Un_Register_SDP_Record()   */
   /*          function.                                                */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
long _MAP_Register_Message_Notification_Server_SDP_Record(unsigned int MAPID, char *ServiceName)
{
   long         ret_val;
   Byte_t       EIRData[2 + UUID_16_SIZE];
   DWord_t      SDPServiceRecordHandle;
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Register_Message_Notification_Server_SDP_Record(_BluetoothStackID, MAPID, ServiceName, &SDPServiceRecordHandle))
      {
         /* Return the SDP Record Handle.                               */
         ret_val = (long)SDPServiceRecordHandle;

         /* Configure the EIR Data.                                     */
         EIRDataLength = NON_ALIGNED_BYTE_SIZE + UUID_16_SIZE;

         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
         ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

         /* Assign the MNS Role UUID (in big-endian format).            */
         SDP_ASSIGN_MESSAGE_NOTIFICATION_SERVER_UUID_16(tempUUID);

         /* Convert the UUID to little endian as required by EIR data.  */
         CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), tempUUID);

         /* Increment the length we pass to the internal function to    */
         /* take into account the length byte.                          */
         EIRDataLength += NON_ALIGNED_BYTE_SIZE;

         /* Configure the EIR data.                                     */
         MOD_AddEIRData(EIRDataLength, EIRData);
      }
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %ld\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for deleting a previously   */
   /* registered MAP SDP Service Record.  This function accepts as its  */
   /* parameter the SDP Service Record Handle of the SDP Service Record */
   /* to delete from the SDP Database.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _MAP_Un_Register_SDP_Record(unsigned int MAPID, DWord_t ServiceRecordHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* Service Record Handle has been specified.                         */
   if((Initialized) && (ServiceRecordHandle))
   {
      /* Simply delete the Service Record Handle.                       */
      MAP_Un_Register_SDP_Record(_BluetoothStackID, MAPID, ServiceRecordHandle);

      /* Flag success to the caller.                                    */
      ret_val = 0;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for opening a connection to */
   /* a remote Message Access Server.  The first parameter to this      */
   /* function is the remote Bluetooth Device Address of the Bluetooth  */
   /* MAP Profile Server with which to connect.  The second parameter   */
   /* specifies the remote server port with which to connect.  The      */
   /* ServerPort parameter *MUST* be between MAP_PORT_NUMBER_MINIMUM and*/
   /* MAP_PORT_NUMBER_MAXIMUM.  This function returns a positive, non   */
   /* zero, value if successful or a negative return error code if an   */
   /* error occurs.  A successful return code will be a MAP ID that can */
   /* be used to reference the remote opened MAP Profile Server in ALL  */
   /* other MAP Profile Client functions in this module.  Once a remote */
   /* server is opened, it can only be closed via a call to the         */
   /* _MAP_Close_Connection() function (passing the return value from   */
   /* this function).                                                   */
int _MAP_Open_Remote_Message_Access_Server_Port(BD_ADDR_t BD_ADDR, unsigned int ServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if a semiv-valid Bluetooth address was specified  */
      /* and the port number is in the allowed range.                   */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ServerPort >= SPP_PORT_NUMBER_MINIMUM) && (ServerPort <= SPP_PORT_NUMBER_MAXIMUM))
      {
         ret_val = MAP_Open_Remote_Message_Access_Server_Port(_BluetoothStackID, BD_ADDR, ServerPort, MAP_Event_Callback, 0);

         /* If an error occurred, map it to a valid error code          */
         if(ret_val <= 0)
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for opening a connection to */
   /* a remote Message Notification Server.  The first parameter to this*/
   /* function is the MAP ID of the local Message Access Server making  */
   /* the connection.  The second parameter specifies the remote server */
   /* port with which to connect.  The ServerPort parameter *MUST* be   */
   /* between MAP_PORT_NUMBER_MINIMUM and MAP_PORT_NUMBER_MAXIMUM.  This*/
   /* function returns a positive, non zero, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a MAP ID that can be used to reference the    */
   /* remote opened MAP Profile Server in ALL other MAP Notification    */
   /* Client functions in this module.  Once a remote server is opened, */
   /* it can only be closed via a call to the _MAP_Close_Connection()   */
   /* function (passing the return value from this function).           */
int _MAP_Open_Remote_Message_Notification_Server_Port(unsigned int LocalMAPID, unsigned int ServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the port is in the allowed range.              */
      if((ServerPort >= SPP_PORT_NUMBER_MINIMUM) && (ServerPort <= SPP_PORT_NUMBER_MAXIMUM))
      {
         ret_val = MAP_Open_Remote_Message_Notification_Server_Port(_BluetoothStackID, LocalMAPID, ServerPort, MAP_Event_Callback, 0);

         /* If an error occurred, map it to a valid error code          */
         if(ret_val <= 0)
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for closing a currently     */
   /* open/registered Message Access Profile server.  This function is  */
   /* capable of closing servers opened via a call to                   */
   /* _MAP_Open_Message_Access_Server() and                             */
   /* _MAP_Open_Message_Notification_Server().  The parameter to this   */
   /* function is the MAP ID of the Profile Server to be closed.  This  */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
int _MAP_Close_Server(unsigned int MAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Close_Server(_BluetoothStackID, MAPID))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for closing a currently     */
   /* ongoing MAP Profile connection.  The parameter to this function is*/
   /* the MAP ID of the MAP Profile connection to be closed.  This      */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* * NOTE * If this function is called with a Server MAP ID (a value */
   /*          returned from a call to _MAP_Open_Server_Port()) any     */
   /*          clients currently connected to this server will be       */
   /*          terminated, but the server will remained open and        */
   /*          registered.  If this function is called using a Client   */
   /*          MAP ID (a value returned from a call to                  */
   /*          _MAP_Open_Remote_Server_Port()), the client connection   */
   /*          will be terminated/closed entirely.                      */
int _MAP_Close_Connection(unsigned int MAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter, MAP ID: %d\n", MAPID));

   /* Check to see if the module has been initialized.                  */
   if(Initialized)
   {
      /* Check to see if semi-valid MAPID was specified.                */
      if(MAPID)
      {
         /* Call Bluetopia MAP close connection function.               */
         ret_val = MAP_Disconnect_Request(_BluetoothStackID, MAPID);

         /* If an error occurred, map it to a valid error code          */
         if(ret_val)
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_DISCONNECT_DEVICE;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Server.  The MAPID parameter specifies the MAP ID   */
   /* for the local MAP Client.  This function returns zero if          */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
int _MAP_Abort_Request(unsigned int MAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Abort_Request(_BluetoothStackID, MAPID))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for providing a mechanism to*/
   /* Enable or Disable Notification messages from the remote Message   */
   /* Access Server.  The first parameter to this function is the       */
   /* MAP ID of the Service Client making this call.  The second        */
   /* parameter specifies if the Notifications should be Enabled or     */
   /* Disabled.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.                               */
int _MAP_Set_Notification_Registration_Request(unsigned int MAPID, Boolean_t Enabled)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Set_Notification_Registration_Request(_BluetoothStackID, MAPID, Enabled))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Notification  */
   /* Registration Response to the remote Client.  The first parameter  */
   /* to this function is the MAP ID of the Server making this call.    */
   /* The second parameter to this function is the Response Code to be  */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int _MAP_Set_Notification_Registration_Response(unsigned int MAPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Set_Notification_Registration_Response(_BluetoothStackID, MAPID, ResponseCode))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for providing a mechanism   */
   /* for Message Notification Clients to dispatch an event to the      */
   /* remote Server.  The first parameter to this function is the MAP ID*/
   /* of the Service Client making this call.  The second parameter     */
   /* specifies the number of bytes that are present in the object      */
   /* segment (DataBuffer) The third parameter in a pointer to the      */
   /* segment of the event object data.  The final parameter to this    */
   /* function is a Boolean Flag indicating if this is to be the final  */
   /* segment of the Event Object.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The EventObject is a "x-bt/MAP-event-report" character   */
   /*          stream that is formatted as defined in the Message Access*/
   /*          Profile Specification.                                   */
int _MAP_Send_Event_Request(unsigned int MAPID, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Send_Event_Request(_BluetoothStackID, MAPID, DataLength, DataBuffer, AmountSent, Final))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Send Event    */
   /* Response to the remote Client.  The first parameter to this       */
   /* function is the MAP ID of the Server making this call.  The second*/
   /* parameter to this function is the Response Code to be associated  */
   /* with this response.  This function returns zero if successful, or */
   /* a negative return value if there was an error.                    */
int _MAP_Send_Event_Response(unsigned int MAPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Send_Event_Response(_BluetoothStackID, MAPID, ResponseCode))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Get Folder Listing Request */
   /* to the specified remote MAP Server.  The MAPID parameter specifies*/
   /* the MAP ID for the local MAP Client (returned from a successful   */
   /* call to the _MAP_Connect_Remote_Server_Port() function).  The     */
   /* MaxListCount parameter is an unsigned integer that specifies the  */
   /* maximum number of list entries the client can handle.  A          */
   /* MaxListCount of ZERO (0) indicates that this is a request for the */
   /* number accessible folders in the current folder.  The             */
   /* ListStartOffset parameter specifies the index requested by the    */
   /* Client in this Folder Listing.  This function returns zero if     */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAP Profile Request    */
   /*          active at any one time.  Because of this, another MAP    */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the _MAP_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the MAP   */
   /*          Profile Event Callback that was registered when the MAP  */
   /*          Profile Port was opened).                                */
int _MAP_Get_Folder_Listing_Request(unsigned int MAPID, Word_t MaxListCount, Word_t ListStartOffset)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Get_Folder_Listing_Request(_BluetoothStackID, MAPID, MaxListCount, ListStartOffset))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Get Folder Listing Response to */
   /* the specified remote MAP Client.  This is used for responding to a*/
   /* MAP Get Folder Listing Indication.  The MAPID parameter specifies */
   /* the MAP ID of the MAP Server responding to the request.  The      */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  The FolderCount parameter is a pointer to a variable   */
   /* that can optionally contain the number of accessible folder       */
   /* entries in the current folder.  This should ONLY be used if the   */
   /* received indication indicated a request for Folder Listing by     */
   /* indicating a MaxListCount = ZERO (0).  In all other instances this*/
   /* parameter must be set to NULL.  The DataLength parameter indicates*/
   /* the number of bytes that are contained in the object segment      */
   /* (DataBuffer).  The DataBuffer parameter is a pointer to a byte    */
   /* buffer containing a segment of the Folder Listing Object.  The    */
   /* AmountSent parameter is a pointer to variable which will be       */
   /* written with the actual amount of data that was able to be        */
   /* included in the packet.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The FolderListingObject is a "x-obex/folder-listing"     */
   /*          character stream that is formatted as defined in the     */
   /*          IrOBEX Specification.                                    */
   /* * NOTE * If FolderCount is not NULL, then the remaining parameters*/
   /*          are ignored.                                             */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
int _MAP_Get_Folder_Listing_Response(unsigned int MAPID, Byte_t ResponseCode, Word_t *FolderCount, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Get_Folder_Listing_Response(_BluetoothStackID, MAPID, ResponseCode, FolderCount, DataLength, DataBuffer, AmountSent))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Get Message Listing Request*/
   /* to the specified remote MAP Server.  The MAPID parameter specifies*/
   /* the MAP ID for the local MAP Client (returned from a successful   */
   /* call to the _MAP_Connect_Remote_Server_Port() function).  The     */
   /* FolderName specifies the Folder from which the Message Listing is */
   /* to be retrieved.  If the parameter is NULL, the listing is made   */
   /* from the current directory.  The MaxListCount parameter is an     */
   /* unsigned integer that specifies the maximum number of list entries*/
   /* the client can handle.  A MaxListCount of ZERO (0) indicates that */
   /* this is a request for the number of messages in the specified     */
   /* folder.  The ListStartOffset parameter specifies the index        */
   /* requested by the Client in this Listing.  The ListInfo structure  */
   /* is used to specify a number of filters and options that should be */
   /* applied to the request.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
int _MAP_Get_Message_Listing_Request(unsigned int MAPID, Word_t *FolderName, Word_t MaxListCount, Word_t ListStartOffset, MAP_Message_Listing_Info_t *ListingInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Get_Message_Listing_Request(_BluetoothStackID, MAPID, FolderName, MaxListCount, ListStartOffset, ListingInfo))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Get Message Listing Response to*/
   /* the specified remote MAP Client.  This is used for responding to a*/
   /* MAP Get Message Listing Indication.  The MAPID parameter specifies*/
   /* the MAP ID of the MAP Server responding to the request.  The      */
   /* ResponseCode parameter is the OBEX response code to include in the*/
   /* response.  The MessageCount parameter contains the number of      */
   /* accessible message entries.  The NewMessage parameter indicates if*/
   /* new messages have arrived since last checked.  The CurrentTime    */
   /* parameters indicates the time at which the response is being sent.*/
   /* The DataLength parameter defines the number of bytes that are     */
   /* included in the object segment (DataBuffer).  The DataBuffer      */
   /* parameter is a pointer to a byte buffer containing a segment of   */
   /* the Message Listing Object.  The AmountSent parameter is a pointer*/
   /* to variable which will be written with the actual amount of data  */
   /* that was able to be included in the packet.  This function returns*/
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The MessageListingObject is a "x-bt/MAP-msg-listing"     */
   /*          character stream that is formatted as defined in the     */
   /*          Message Access Profile Specification.                    */
   /* * NOTE * If MessageCount is not NULL, then the remaining          */
   /*          parameters are ignored.                                  */
   /* * NOTE * CurrentTime is used by the receiver of this response to  */
   /*          correlate the timestamps in the listing with the current */
   /*          time of the server.                                      */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
int _MAP_Get_Message_Listing_Response(unsigned int MAPID, Byte_t ResponseCode, Word_t *MessageCount, Boolean_t NewMessage, MAP_TimeDate_t *CurrentTime, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Get_Message_Listing_Response(_BluetoothStackID, MAPID, ResponseCode, MessageCount, NewMessage, CurrentTime, DataLength, DataBuffer, AmountSent))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Get Message Request to the */
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  The MessageHandle parameter is a*/
   /* pointer to a 16 byte NULL terminated Unicode Text string that     */
   /* identifies the message.  The Attachment parameter is used to      */
   /* indicate if any existing attachments to the specified message are */
   /* to be included in the response.  The CharSet and Fractional Type  */
   /* parameters specify that format of the response message.  This     */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.                          */
int _MAP_Get_Message_Request(unsigned int MAPID, char *MessageHandle, Boolean_t Attachment, MAP_CharSet_t CharSet, MAP_Fractional_Type_t FractionalType)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Get_Message_Request(_BluetoothStackID, MAPID, MessageHandle, Attachment, CharSet, FractionalType))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Get Message Response to the    */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Get Message Indication.  The MAPID parameter specifies the MAP ID */
   /* of the MAP Server responding to the request.  The ResponseCode    */
   /* parameter is the OBEX response code to include in the response.   */
   /* The FractionalType parameter indicates the type of the object     */
   /* segment and can specify ftUnfragmented, ftMore or ftLast.  The    */
   /* DataLength parameter defines the number of bytes that are included*/
   /* in the object segment (DataBuffer).  The DataBuffer parameter is a*/
   /* pointer to a byte buffer containing the Message Listing Object    */
   /* segment.  The AmountSent parameter is a pointer to variable which */
   /* will be written with the actual amount of data that was able to be*/
   /* included in the packet.  This function returns zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The MessageObject is a "x-bt/message" character stream   */
   /*          that is formatted as defined in the Message Access       */
   /*          Profile Specification.                                   */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
   /* * NOTE * Specifying the FractionalType as ftUnfragmented causes no*/
   /*          FractionalType Header to be added to the OBEX Header     */
   /*          List.  This is the value that should be specified for a a*/
   /*          message that is non-fragmented.  Note that if the Get    */
   /*          Message Indication specified a non-fragmented            */
   /*          FractionalType then you must respond with the correct    */
   /*          non-fragmented FractionalType (i.e. ftMore or ftLast).   */
int _MAP_Get_Message_Response(unsigned int MAPID, Byte_t ResponseCode, MAP_Fractional_Type_t FractionalType, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Get_Message_Response(_BluetoothStackID, MAPID, ResponseCode, FractionalType, DataLength, DataBuffer, AmountSent))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Set Message Status Request */
   /* to the specified remote MAP Server.  The MAPID parameter specifies*/
   /* the MAP ID for the local MAP Client.  The MessageHandle parameter */
   /* is a pointer to a 16 byte NULL terminated Unicode Text string that*/
   /* identifies the message.  The StatusIndicator identifies the Status*/
   /* indicator to set.  The StatusValue indicates the new state of the */
   /* indicator.  This function returns zero if successful or a negative*/
   /* return error code if there was an error.                          */
int _MAP_Set_Message_Status_Request(unsigned int MAPID, char *MessageHandle, MAP_Status_Indicator_t StatusIndicator, Boolean_t StatusValue)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID and Message Handle has been specified.                     */
   if((Initialized) && (MAPID) && (MessageHandle))
   {
      if(!MAP_Set_Message_Status_Request(_BluetoothStackID, MAPID, MessageHandle, StatusIndicator, StatusValue))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Set Message Status Response to */
   /* the specified remote MAP Client.  This is used for responding to a*/
   /* MAP Set Message Status Indication.  The MAPID parameter specifies */
   /* the MAP ID for the local MAP Client.  The ResponseCode parameter  */
   /* is the OBEX response code to include in the response.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _MAP_Set_Message_Status_Response(unsigned int MAPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Set_Message_Status_Response(_BluetoothStackID, MAPID, ResponseCode))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Push Message Request to the*/
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  The FolderName parameter        */
   /* specifies the destination location for the message.  If this      */
   /* parameter is NULL, the current directory is used.  The Transparent*/
   /* parameter is used to indicate that no copy of the message should  */
   /* be placed in the Sent Folder.  Retry parameter is used to indicate*/
   /* if any attempts to retry the send if the previous attempt fails.  */
   /* The CharSet parameters specify that format of the message.  The   */
   /* DataLength parameter defines the number of bytes that are included*/
   /* in the object segment (DataBuffer).  The DataBuffer parameter is a*/
   /* pointer to a byte buffer containing the Message Listing Object    */
   /* segment.  The AmountSent parameter is a pointer to variable which */
   /* will be written with the actual amount of data that was able to be*/
   /* included in the packet.  The final parameter to this function is a*/
   /* Boolean Flag indicating if this is the last segment of the object.*/
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The MessageObject is a "x-bt/message" character stream   */
   /*          that is formatted as defined in the Message Access       */
   /*          Profile Specification.                                   */
   /* * NOTE * Including a DataBuffer pointer and setting DataLength > 0*/
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          If the stack cannot include all the requested object     */
   /*          (DataLength) in the current packet, a Body header will be*/
   /*          used and AmountSent will reflect that not all of the data*/
   /*          was sent.  If all data is included, an End-of-Body header*/
   /*          will be used.                                            */
   /* * NOTE * If AmountSent returns an amount smaller than the         */
   /*          specified DataLength, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted DataLength and DataBuffer pointer to account for*/
   /*          the data that was successfully sent.                     */
int _MAP_Push_Message_Request(unsigned int MAPID, Word_t *FolderName, Boolean_t Transparent, Boolean_t Retry, MAP_CharSet_t CharSet, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountSent, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Push_Message_Request(_BluetoothStackID, MAPID, FolderName, Transparent, Retry, CharSet, DataLength, DataBuffer, AmountSent, Final))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Push Message Response to the   */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Push Message Indication.  The MAPID parameter specifies the MAP ID*/
   /* of the MAP Server responding to the request.  The ResponseCode    */
   /* parameter is the OBEX response code to include in the response.   */
   /* The MessageHandle parameter points to an ASCII string that        */
   /* contains 16 hexadecimal digits and represents the handle assigned */
   /* to the message pushed.                                            */
   /* * NOTE * The MessageHandle pointer must point to a valid string if*/
   /*          the Response Code indicates MAP_OBEX_RESPONSE_OK,        */
   /*          otherwise it may be NULL.                                */
int _MAP_Push_Message_Response(unsigned int MAPID, Byte_t ResponseCode, char *MessageHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and the parameters*/
   /* appear to be semi-valid.                                          */
   if((Initialized) && (MAPID) && ((ResponseCode != MAP_OBEX_RESPONSE_OK) || ((ResponseCode == MAP_OBEX_RESPONSE_OK) && (MessageHandle))))
   {
      if(!MAP_Push_Message_Response(_BluetoothStackID, MAPID, ResponseCode, MessageHandle))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Update Inbox Request to the*/
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _MAP_Update_Inbox_Request(unsigned int MAPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Update_Inbox_Request(_BluetoothStackID, MAPID))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Update Inbox Response to the   */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Set Message Status Indication.  The MAPID parameter specifies the */
   /* MAP ID for the local MAP Client.  The ResponseCode parameter is   */
   /* the OBEX response code to include in the response.  This function */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _MAP_Update_Inbox_Response(unsigned int MAPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and MAP ID has    */
   /* been specified.                                                   */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Update_Inbox_Response(_BluetoothStackID, MAPID, ResponseCode))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function generates a MAP Set Folder Request to the  */
   /* specified remote MAP Server.  The MAPID parameter specifies the   */
   /* MAP ID for the local MAP Client.  The PathOption parameter        */
   /* contains an enumerated value that indicates the type of path      */
   /* change to request.  The FolderName parameter contains the folder  */
   /* name to include with this SetFolder request.  This value can be   */
   /* NULL if no name is required for the selected PathOption.  See the */
   /* MAP specification for more information.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          MAP Profile Server successfully processed the command.   */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote MAP Profile Server successfully  */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding MAP Profile Request    */
   /*          active at any one time.  Because of this, another MAP    */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the MAP_Abort_Request()   */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the MAP   */
   /*          Profile Event Callback that was registered when the MAP  */
   /*          Profile Port was opened).                                */
int _MAP_Set_Folder_Request(unsigned int MAPID, MAP_Set_Folder_Option_t PathOption, Word_t *FolderName)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if(Initialized)
   {
      if(MAPID)
      {
         if(!MAP_Set_Folder_Request(_BluetoothStackID, MAPID, PathOption, FolderName))
            ret_val = 0;
         else
            ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function sends a MAP Set Folder Response to the     */
   /* specified remote MAP Client.  This is used for responding to a MAP*/
   /* Set Folder Indication.  The MAPID parameter specifies the MAP ID  */
   /* for the local MAP Client.  The ResponseCode parameter is the OBEX */
   /* response code to include in the response.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _MAP_Set_Folder_Response(unsigned int MAPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized and a semi-valid  */
   /* MAP ID has been specified.                                        */
   if((Initialized) && (MAPID))
   {
      if(!MAP_Set_Folder_Response(_BluetoothStackID, MAPID, ResponseCode))
         ret_val = 0;
      else
         ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION;
   }
   else
      ret_val = BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_MESSAGE_ACCESS | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

