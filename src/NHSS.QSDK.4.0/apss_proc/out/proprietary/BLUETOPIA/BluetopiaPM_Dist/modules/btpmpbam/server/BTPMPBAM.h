/*****< btpmpbam.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPBAM - Phone Book Access Manager for Stonestreet One Bluetooth        */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMPBAMH__
#define __BTPMPBAMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "PBAMAPI.h"             /* PBAM Manager API Prototypes/Constants.    */

#include "SS1BTPBA.h"            /* PBAP Framework Prototypes/Constants.      */

   /* The following type is used with the PBAM_Update_Data_t structure  */
   /* (which is used with the PBAM_NotifyUpdate() function to inform the*/
   /* Hands Free Manager that an Update needs to be dispatched.         */
typedef enum
{
   utPhoneBookAccessEvent
} PBAM_Update_Type_t;

typedef struct _tagPBAM_Phone_Book_Access_Event_Data_t
{
   PBAP_Event_Type_t EventType;
   union
   {
      PBAP_Open_Port_Confirmation_Data_t          OpenPortConfirmationData;
      PBAP_Close_Port_Indication_Data_t           ClosePortIndicationData;
      PBAP_Abort_Confirmation_Data_t              AbortConfirmationData;
      PBAP_Pull_Phonebook_Confirmation_Data_t     PullPhonebookConfirmationData;
      PBAP_Set_Phonebook_Confirmation_Data_t      SetPhonebookConfirmationData;
      PBAP_Pull_vCard_Listing_Confirmation_Data_t PullvCardListingConfirmationData;
      PBAP_Pull_vCard_Entry_Confirmation_Data_t   PullvCardEntryConfirmationData;

      PBAP_Open_Port_Request_Indication_Data_t    OpenPortRequestIndicationData;
      PBAP_Open_Port_Indication_Data_t            OpenPortIndicationData;
      PBAP_Abort_Indication_Data_t                AbortIndicationData;
      PBAP_Pull_Phonebook_Indication_Data_t       PullPhonebookIndicationData;
      PBAP_Set_Phonebook_Indication_Data_t        SetPhonebookIndicationData;
      PBAP_Pull_vCard_Listing_Indication_Data_t   PullvCardListingIndicationData;
      PBAP_Pull_vCard_Entry_Indication_Data_t     PullvCardEntryIndicationData;
   } EventData;
} PBAM_Phone_Book_Access_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of update needs to be dispatched (used*/
   /* with the PBAM_NotifyUpdate() function).                           */
typedef struct _tagPBAM_UpdateData_t
{
   PBAM_Update_Type_t UpdateType;
   union
   {
      PBAM_Phone_Book_Access_Event_Data_t PhoneBookAccessEventData;
   } UpdateData;
} PBAM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Phone Book Access Manager of a specific Update      */
   /* Event. The Phone Book Access Manager can then take the correct    */
   /* action to process the update.                                     */
Boolean_t PBAM_NotifyUpdate(PBAM_Update_Data_t *UpdateData);

#endif
