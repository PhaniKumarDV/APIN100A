/*****< CPPMUTIL.h >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMUTIL - Cycling Power Platform Manager Client Utilities Header         */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/
#ifndef __CPPMUTILH__
#define __CPPMUTILH__

   /* The Callback_Entry_t structure defines the elements of a list     */
   /* of callback information that can used to implement more than one  */
   /* registered callback in the same client application.               */
typedef struct _tagCallback_Entry_t
{
   unsigned int                 CallbackID;
   CPPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagCallback_Entry_t *NextCallbackEntry;
} Callback_Entry_t;

   /* AddCallbackEntry adds an entry to supplied callback list          */
   /* verifying that no entry with the same callback ID is already      */
   /* present.                                                          */
Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **CallbackEntryList, Callback_Entry_t *EntryToAdd);

   /* SearchCallbackEntry searches the callback list for an entry with  */
   /* the supplied callback ID and returns it if one is found.          */
Callback_Entry_t *SearchCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID);

   /* DeleteCallbackEntry removes an entry from the callback list with  */
   /* the supplied callback ID if one is found. A pointer to the entry  */
   /* is returned. The returned entry can be freed with                 */
   /* FreeCallbackEntryMemory.                                          */
Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID);

   /* FreeCallbackEntryList frees the memory of every entry in the      */
   /* supplied callback list.                                           */
void FreeCallbackEntryList(Callback_Entry_t **CallbackEntryList);

   /* FreeCallbackEntryMemory frees the memory of the supplied callback */
   /* entry.                                                            */
void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);

#endif
