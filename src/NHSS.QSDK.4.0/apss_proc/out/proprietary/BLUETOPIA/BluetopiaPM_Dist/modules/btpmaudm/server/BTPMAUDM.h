/*****< btpmaudm.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMAUDM - Audio Manager for Stonestreet One Bluetooth Protocol Stack     */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMAUDMH__
#define __BTPMAUDMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "AUDMAPI.h"             /* BTPM Audio Manager Prototypes/Constants.  */

   /* The following type is used with the AUDM_Update_Data_t structure  */
   /* (which is used with the AUDM_NotifyUpdate() function to inform the*/
   /* Audio Manager that an Update needs to be dispatched.              */
typedef enum
{
   utAUDEvent
} AUDM_Update_Type_t;

   /* The following structure is used with utAUDEvent Remote Control    */
   /* Command Indication events.                                        */
   /* * NOTE * If any members are added to this structure that the      */
   /*          Message member must be the last member.  This is due to  */
   /*          the fact that the message structure contains an array of */
   /*          variable-sized data as the last member the structure.    */
   /*          The message must be last here so that data written to the*/
   /*          variable-sized array does not overwrite other members in */
   /*          the structure.                                           */
typedef struct _tagAUDM_AUD_Remote_Control_Command_Indication_Data_t
{
   AUD_Remote_Control_Command_Indication_Data_t   RemoteControlCommandIndicationData;
   AUDM_Remote_Control_Command_Received_Message_t Message;
} AUDM_AUD_Remote_Control_Command_Indication_Data_t;

   /* The following structure is used with utAUDEvent Remote Control    */
   /* Command Confirmation events.                                      */
   /* * NOTE * If any members are added to this structure that the      */
   /*          Message member must be the last member.  This is due to  */
   /*          the fact that the message structure contains an array of */
   /*          variable-sized data as the last member the structure.    */
   /*          The message must be last here so that data written to the*/
   /*          variable-sized array does not overwrite other members in */
   /*          the structure.                                           */
typedef struct _tagAUDM_AUD_Remote_Control_Command_Confirmation_Data_t
{
   AUD_Remote_Control_Command_Confirmation_Data_t RemoteControlCommandConfirmationData;
   AUDM_Remote_Control_Command_Status_Message_t   Message;
} AUDM_AUD_Remote_Control_Command_Confirmation_Data_t;

typedef struct _tagAUDM_AUD_Event_Data_t
{
   AUD_Event_Type_t EventType;
   union
   {
      AUD_Open_Request_Indication_Data_t                  OpenRequestIndicationData;
      AUD_Stream_Open_Indication_Data_t                   StreamOpenIndicationData;
      AUD_Stream_Open_Confirmation_Data_t                 StreamOpenConfirmationData;
      AUD_Stream_Close_Indication_Data_t                  StreamCloseIndicationData;
      AUD_Stream_State_Change_Indication_Data_t           StreamStateChangeIndicationData;
      AUD_Stream_State_Change_Confirmation_Data_t         StreamStateChangeConfirmationData;
      AUD_Stream_Format_Change_Indication_Data_t          StreamFormatChangeIndicationData;
      AUD_Stream_Format_Change_Confirmation_Data_t        StreamFormatChangeConfirmationData;
      AUD_Encoded_Audio_Data_Indication_Data_t            EncodedAudioDataIndicationData;
      AUDM_AUD_Remote_Control_Command_Indication_Data_t   AUDMRemoteControlCommandIndicationData;
      AUDM_AUD_Remote_Control_Command_Confirmation_Data_t AUDMRemoteControlCommandConfirmationData;
      AUD_Remote_Control_Open_Indication_Data_t           RemoteControlOpenIndicationData;
      AUD_Remote_Control_Open_Confirmation_Data_t         RemoteControlOpenConfirmationData;
      AUD_Remote_Control_Close_Indication_Data_t          RemoteControlCloseIndicationData;
      AUD_Browsing_Channel_Open_Indication_Data_t         BrowsingChannelOpenIndicationData;
      AUD_Browsing_Channel_Open_Confirmation_Data_t       BrowsingChannelOpenConfirmationData;
      AUD_Browsing_Channel_Close_Indication_Data_t        BrowsingChannelCloseIndicationData;
   } EventData;
} AUDM_AUD_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the AUDM_NotifyUpdate() function).                           */
typedef struct _tagAUDM_Update_Data_t
{
   AUDM_Update_Type_t UpdateType;
   union
   {
      AUDM_AUD_Event_Data_t AUDEventData;
   } UpdateData;
} AUDM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Audio Manager of a specific Update Event.  The Audio*/
   /* Manager can then take the correct action to process the update.   */
Boolean_t AUDM_NotifyUpdate(AUDM_Update_Data_t *UpdateData);

#endif
