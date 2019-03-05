/*****< CPPMUTIL.c >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMUTIL - Cycling Power Platform Manager Client Utility Functions        */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/

#include "SS1BTCPS.h"
#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "CPPMType.h"
#include "CPPMAPI.h"             /* CPP Manager Prototypes/Constants.         */
#include "CPPMUTIL.h"            /* CPP Client Utility Header                 */


   /* AddCallbackEntry adds an entry to supplied callback list          */
   /* verifying that no entry with the same callback ID is already      */
   /* present.                                                          */
Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **CallbackEntryList, Callback_Entry_t *EntryToAdd)
{
   Callback_Entry_t *AddedEntry = NULL;
   Callback_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToAdd)
   {
      AddedEntry = (Callback_Entry_t *)BTPS_AllocateMemory(sizeof(Callback_Entry_t));

      if(AddedEntry)
      {
         /* Copy the data.                                              */
         *AddedEntry                     = *EntryToAdd;

         AddedEntry->NextCallbackEntry = NULL;

         /* Check for an empty list.                                    */
         if((tmpEntry = *CallbackEntryList) != NULL)
         {
            /* Traverse the list.                                       */
            while(tmpEntry)
            {
               if(tmpEntry->CallbackID == AddedEntry->CallbackID)
               {
                  /* The Entry is already present.                      */
                  BTPS_FreeMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* Check for the end of the list.                     */
                  if(tmpEntry->NextCallbackEntry)
                     tmpEntry = tmpEntry->NextCallbackEntry;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Add the entry.                                        */
               tmpEntry->NextCallbackEntry = AddedEntry;
            }
         }
         else
            *CallbackEntryList = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* SearchCallbackEntry searches the callback list for an entry with  */
   /* the supplied callback ID and returns it if one is found.          */
Callback_Entry_t *SearchCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID)
{
   Callback_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   FoundEntry = *CallbackEntryList;

   while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      FoundEntry = FoundEntry->NextCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* DeleteCallbackEntry removes an entry from the callback list with  */
   /* the supplied callback ID if one is found. A pointer to the entry  */
   /* is returned. The returned entry can be freed with                 */
   /* FreeCallbackEntryMemory.                                          */
Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID)
{
   Callback_Entry_t *FoundEntry = NULL;
   Callback_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   FoundEntry = *CallbackEntryList;

   while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
   {
      LastEntry  = FoundEntry;
      FoundEntry = FoundEntry->NextCallbackEntry;
   }

   /* Check for the specified entry.                                    */
   if(FoundEntry)
   {
      if(LastEntry)
      {
         /* Entry was not the first entry in the list.                  */
         LastEntry->NextCallbackEntry = FoundEntry->NextCallbackEntry;
      }
      else
         *CallbackEntryList = FoundEntry->NextCallbackEntry;

      FoundEntry->NextCallbackEntry = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* FreeCallbackEntryList frees the memory of every entry in the      */
   /* supplied callback list.                                           */
void FreeCallbackEntryList(Callback_Entry_t **CallbackEntryList)
{
   Callback_Entry_t *EntryToFree;
   Callback_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   EntryToFree = *CallbackEntryList;

   while(EntryToFree)
   {
      tmpEntry    = EntryToFree;
      EntryToFree = EntryToFree->NextCallbackEntry;

      FreeCallbackEntryMemory(tmpEntry);
   }

   *CallbackEntryList = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* FreeCallbackEntryMemory frees the memory of the supplied callback */
   /* entry.                                                            */
void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}
