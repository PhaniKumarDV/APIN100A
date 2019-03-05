/*****< btpmmapm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMAPM - Massage Access Manager for Stonestreet One Bluetooth           */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/24/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMMAPMH__
#define __BTPMMAPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "MAPMAPI.h"             /* MAPM Manager API Prototypes/Constants.    */

   /* The following type is used with the MAPM_Update_Data_t structure  */
   /* (which is used with the MAPM_NotifyUpdate() function to inform the*/
   /* Message Access Manager that an Update needs to be dispatched.     */
typedef enum
{
   utMAPEvent
} MAPM_Update_Type_t;

typedef struct _tagMAPM_MAP_Event_Data_t
{
   MAP_Event_Type_t EventType;
   union
   {
      MAP_Open_Request_Indication_Data_t                     OpenRequestIndicationData;
      MAP_Open_Port_Indication_Data_t                        OpenPortIndicationData;
      MAP_Open_Port_Confirmation_Data_t                      OpenPortConfirmationData;
      MAP_Close_Port_Indication_Data_t                       ClosePortIndicationData;
      MAP_Notification_Registration_Indication_Data_t        NotificationRegistrationIndicationData;
      MAP_Notification_Registration_Confirmation_Data_t      NotificationRegistrationConfirmationData;
      MAP_Send_Event_Indication_Data_t                       SendEventIndicationData;
      MAP_Send_Event_Confirmation_Data_t                     SendEventConfirmationData;
      MAP_Get_Folder_Listing_Indication_Data_t               GetFolderListingIndicationData;
      MAP_Get_Folder_Listing_Confirmation_Data_t             GetFolderListingConfirmationData;
      MAP_Get_Message_Listing_Indication_Data_t              GetMessageListingIndicationData;
      MAP_Get_Message_Listing_Confirmation_Data_t            GetMessageListingConfirmationData;
      MAP_Get_Message_Indication_Data_t                      GetMessageIndicationData;
      MAP_Get_Message_Confirmation_Data_t                    GetMessageConfirmationData;
      MAP_Set_Message_Status_Indication_Data_t               SetMessageStatusIndicationData;
      MAP_Set_Message_Status_Confirmation_Data_t             SetMessageStatusConfirmationData;
      MAP_Push_Message_Indication_Data_t                     PushMessageIndicationData;
      MAP_Push_Message_Confirmation_Data_t                   PushMessageConfirmationData;
      MAP_Update_Inbox_Indication_Data_t                     UpdateInboxIndicationData;
      MAP_Update_Inbox_Confirmation_Data_t                   UpdateInboxConfirmationData;
      MAP_Set_Folder_Indication_Data_t                       SetFolderIndicationData;
      MAP_Set_Folder_Confirmation_Data_t                     SetFolderConfirmationData;
      MAP_Abort_Indication_Data_t                            AbortIndicationData;
      MAP_Abort_Confirmation_Data_t                          AbortConfirmationData;
   } EventData;
} MAPM_MAP_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of update needs to be dispatched (used*/
   /* with the MAPM_NotifyUpdate() function).                           */
typedef struct _tagMAPM_UpdateData_t
{
   MAPM_Update_Type_t UpdateType;
   union
   {
      MAPM_MAP_Event_Data_t MAPEventData;
   } UpdateData;
} MAPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Message Access Manager of a specific Update Event.  */
   /* The Message Access Manager can then take the correct action to    */
   /* process the update.                                               */
Boolean_t MAPM_NotifyUpdate(MAPM_Update_Data_t *UpdateData);

#endif
