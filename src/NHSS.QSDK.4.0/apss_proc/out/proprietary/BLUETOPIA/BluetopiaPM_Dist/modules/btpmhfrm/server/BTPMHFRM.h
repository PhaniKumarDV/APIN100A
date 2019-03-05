/*****< btpmhfrm.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHFRM - Hands Free Manager for Stonestreet One Bluetooth Protocol      */
/*             Stack Platform Manager.                                        */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMHFRMH__
#define __BTPMHFRMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "HFRMAPI.h"             /* HFRE Manager API Prototypes/Constants.    */

   /* The following type is used with the HFRM_Update_Data_t structure  */
   /* (which is used with the HFRM_NotifyUpdate() function to inform the*/
   /* Hands Free Manager that an Update needs to be dispatched.         */
typedef enum
{
   utHandsFreeEvent
} HFRM_Update_Type_t;

typedef struct _tagHFRM_Hands_Free_Event_Data_t
{
   HFRE_Event_Type_t EventType;
   union
   {
      HFRE_Open_Port_Request_Indication_Data_t                                HFRE_Open_Port_Request_Indication_Data;
      HFRE_Open_Port_Indication_Data_t                                        HFRE_Open_Port_Indication_Data;
      HFRE_Open_Port_Confirmation_Data_t                                      HFRE_Open_Port_Confirmation_Data;
      HFRE_Open_Service_Level_Connection_Indication_Data_t                    HFRE_Open_Service_Level_Connection_Indication_Data;
      HFRE_Control_Indicator_Status_Indication_Data_t                         HFRE_Control_Indicator_Status_Indication_Data;
      HFRE_Control_Indicator_Status_Confirmation_Data_t                       HFRE_Control_Indicator_Status_Confirmation_Data;
      HFRE_Call_Hold_Multiparty_Support_Confirmation_Data_t                   HFRE_Call_Hold_Multiparty_Support_Confirmation_Data;
      HFRE_Call_Hold_Multiparty_Selection_Indication_Data_t                   HFRE_Call_Hold_Multiparty_Selection_Indication_Data;
      HFRE_Call_Waiting_Notification_Activation_Indication_Data_t             HFRE_Call_Waiting_Notification_Activation_Indication_Data;
      HFRE_Call_Waiting_Notification_Indication_Data_t                        HFRE_Call_Waiting_Notification_Indication_Data;
      HFRE_Call_Line_Identification_Notification_Activation_Indication_Data_t HFRE_Call_Line_Identification_Notification_Activation_Indication_Data;
      HFRE_Call_Line_Identification_Notification_Indication_Data_t            HFRE_Call_Line_Identification_Notification_Indication_Data;
      HFRE_Disable_Sound_Enhancement_Indication_Data_t                        HFRE_Disable_Sound_Enhancement_Indication_Data;
      HFRE_Dial_Phone_Number_Indication_Data_t                                HFRE_Dial_Phone_Number_Indication_Data;
      HFRE_Dial_Phone_Number_From_Memory_Indication_Data_t                    HFRE_Dial_Phone_Number_From_Memory_Indication_Data;
      HFRE_ReDial_Last_Phone_Number_Indication_Data_t                         HFRE_ReDial_Last_Phone_Number_Indication_Data;
      HFRE_Ring_Indication_Data_t                                             HFRE_Ring_Indication_Data;
      HFRE_Answer_Call_Indication_Data_t                                      HFRE_Answer_Call_Indication_Data;
      HFRE_InBand_Ring_Tone_Setting_Indication_Data_t                         HFRE_InBand_Ring_Tone_Setting_Indication_Data;
      HFRE_Generate_DTMF_Tone_Indication_Data_t                               HFRE_Generate_DTMF_Tone_Indication_Data;
      HFRE_Voice_Recognition_Notification_Indication_Data_t                   HFRE_Voice_Recognition_Notification_Indication_Data;
      HFRE_Speaker_Gain_Indication_Data_t                                     HFRE_Speaker_Gain_Indication_Data;
      HFRE_Microphone_Gain_Indication_Data_t                                  HFRE_Microphone_Gain_Indication_Data;
      HFRE_Voice_Tag_Request_Indication_Data_t                                HFRE_Voice_Tag_Request_Indication_Data;
      HFRE_Voice_Tag_Request_Confirmation_Data_t                              HFRE_Voice_Tag_Request_Confirmation_Data;
      HFRE_Hang_Up_Indication_Data_t                                          HFRE_Hang_Up_Indication_Data;
      HFRE_Audio_Connection_Indication_Data_t                                 HFRE_Audio_Connection_Indication_Data;
      HFRE_Audio_Disconnection_Indication_Data_t                              HFRE_Audio_Disconnection_Indication_Data;
      HFRE_Audio_Data_Indication_Data_t                                       HFRE_Audio_Data_Indication_Data;
      HFRE_Close_Port_Indication_Data_t                                       HFRE_Close_Port_Indication_Data;
      HFRE_Current_Calls_List_Indication_Data_t                               HFRE_Current_Calls_List_Indication_Data;
      HFRE_Current_Calls_List_Confirmation_Data_t                             HFRE_Current_Calls_List_Confirmation_Data;
      HFRE_Network_Operator_Selection_Format_Indication_Data_t                HFRE_Network_Operator_Selection_Format_Indication_Data;
      HFRE_Network_Operator_Selection_Indication_Data_t                       HFRE_Network_Operator_Selection_Indication_Data;
      HFRE_Network_Operator_Selection_Confirmation_Data_t                     HFRE_Network_Operator_Selection_Confirmation_Data;
      HFRE_Extended_Error_Result_Activation_Indication_Data_t                 HFRE_Extended_Error_Result_Activation_Indication_Data;
      HFRE_Subscriber_Number_Information_Indication_Data_t                    HFRE_Subscriber_Number_Information_Indication_Data;
      HFRE_Subscriber_Number_Information_Confirmation_Data_t                  HFRE_Subscriber_Number_Information_Confirmation_Data;
      HFRE_Response_Hold_Status_Indication_Data_t                             HFRE_Response_Hold_Status_Indication_Data;
      HFRE_Response_Hold_Status_Confirmation_Data_t                           HFRE_Response_Hold_Status_Confirmation_Data;
      HFRE_Incoming_Call_State_Indication_Data_t                              HFRE_Incoming_Call_State_Indication_Data;
      HFRE_Incoming_Call_State_Confirmation_Data_t                            HFRE_Incoming_Call_State_Confirmation_Data;
      HFRE_Command_Result_Data_t                                              HFRE_Command_Result_Data;
      HFRE_Arbitrary_Command_Indication_Data_t                                HFRE_Arbitrary_Command_Indication_Data;
      HFRE_Arbitrary_Response_Indication_Data_t                               HFRE_Arbitrary_Response_Indication_Data;
      HFRE_Available_Codec_List_Indication_Data_t                             HFRE_Available_Codec_List_Indication_Data;
      HFRE_Codec_Select_Indication_t                                          HFRE_Codec_Select_Indication;
      HFRE_Codec_Select_Confirmation_t                                        HFRE_Codec_Select_Confirmation;
      HFRE_Codec_Connection_Setup_Indication_Data_t                           HFRE_Codec_Connection_Setup_Indication_Data;
   } EventData;
} HFRM_Hands_Free_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the HFRM_NotifyUpdate() function).                           */
typedef struct _tagHFRM_Update_Data_t
{
   HFRM_Update_Type_t UpdateType;
   union
   {
      HFRM_Hands_Free_Event_Data_t HandsFreeEventData;
   } UpdateData;
} HFRM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Hands Free Manager of a specific Update Event.  The */
   /* Hands Free Manager can then take the correct action to process the*/
   /* update.                                                           */
Boolean_t HFRM_NotifyUpdate(HFRM_Update_Data_t *UpdateData);

#endif
