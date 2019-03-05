/*****< btpmhdsm.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDSM - Headset Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMHDSMH__
#define __BTPMHDSMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "HDSMAPI.h"             /* HDSET Manager API Prototypes/Constants.   */

   /* The following type is used with the HDSM_Update_Data_t structure  */
   /* (which is used with the HDSM_NotifyUpdate() function to inform the*/
   /* Headset Manager that an Update needs to be dispatched.            */
typedef enum
{
   utHeadsetEvent
} HDSM_Update_Type_t;

typedef struct _tagHDSM_Headset_Event_Data_t
{
   HDSET_Event_Type_t EventType;
   union
   {
      HDSET_Open_Port_Request_Indication_Data_t   HDSET_Open_Port_Request_Indication_Data;
      HDSET_Open_Port_Indication_Data_t           HDSET_Open_Port_Indication_Data;
      HDSET_Open_Port_Confirmation_Data_t         HDSET_Open_Port_Confirmation_Data;
      HDSET_Ring_Indication_Data_t                HDSET_Ring_Indication_Data;
      HDSET_Button_Pressed_Indication_Data_t      HDSET_Button_Pressed_Indication_Data;
      HDSET_Speaker_Gain_Indication_Data_t        HDSET_Speaker_Gain_Indication_Data;
      HDSET_Microphone_Gain_Indication_Data_t     HDSET_Microphone_Gain_Indication_Data;
      HDSET_Audio_Connection_Indication_Data_t    HDSET_Audio_Connection_Indication_Data;
      HDSET_Audio_Disconnection_Indication_Data_t HDSET_Audio_Disconnection_Indication_Data;
      HDSET_Audio_Data_Indication_Data_t          HDSET_Audio_Data_Indication_Data;
      HDSET_Close_Port_Indication_Data_t          HDSET_Close_Port_Indication_Data;
   } EventData;
} HDSM_Headset_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the HDSM_NotifyUpdate() function).                           */
typedef struct _tagHDSM_Update_Data_t
{
   HDSM_Update_Type_t UpdateType;
   union
   {
      HDSM_Headset_Event_Data_t HeadsetEventData;
   } UpdateData;
} HDSM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Headset Manager of a specific Update Event.  The    */
   /* Headset Manager can then take the correct action to process the   */
   /* update.                                                           */
Boolean_t HDSM_NotifyUpdate(HDSM_Update_Data_t *UpdateData);

#endif
