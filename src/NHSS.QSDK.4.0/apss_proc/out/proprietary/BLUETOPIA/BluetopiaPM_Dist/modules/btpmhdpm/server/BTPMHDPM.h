/*****< btpmhdpm.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDPM - Health Device Profile Manager for Stonestreet One Bluetooth    */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMHDPMH__
#define __BTPMHDPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "HDPMAPI.h"             /* HDP Manager API Prototypes/Constants.     */

   /* The following type is used with the HDPM_Update_Data_t structure  */
   /* (which is used with the HDPM_NotifyUpdate() function to inform the*/
   /* Health Device Manager that an Update needs to be dispatched.      */
typedef enum
{
   utHDPEvent
} HDPM_Update_Type_t;

typedef struct _tagHDPM_HDP_Event_Data_t
{
   HDP_Event_Type_t EventType;
   union
   {
      HDP_Connect_Request_Indication_Data_t       HDP_Connect_Request_Indication_Data;
      HDP_Control_Connect_Indication_Data_t       HDP_Control_Connect_Indication_Data;
      HDP_Control_Connect_Confirmation_Data_t     HDP_Control_Connect_Confirmation_Data;
      HDP_Control_Disconnect_Indication_Data_t    HDP_Control_Disconnect_Indication_Data;
      HDP_Control_Create_Data_Link_Indication_t   HDP_Control_Create_Data_Link_Indication_Data;
      HDP_Control_Create_Data_Link_Confirmation_t HDP_Control_Create_Data_Link_Confirmation_Data;
      HDP_Control_Abort_Data_Link_Indication_t    HDP_Control_Abort_Data_Link_Indication_Data;
      HDP_Control_Abort_Data_Link_Confirmation_t  HDP_Control_Abort_Data_Link_Confirmation_Data;
      HDP_Control_Delete_Data_Link_Indication_t   HDP_Control_Delete_Data_Link_Indication_Data;
      HDP_Control_Delete_Data_Link_Confirmation_t HDP_Control_Delete_Data_Link_Confirmation_Data;
      HDP_Data_Link_Connect_Indication_Data_t     HDP_Data_Link_Connect_Indication_Data;
      HDP_Data_Link_Connect_Confirmation_Data_t   HDP_Data_Link_Connect_Confirmation_Data;
      HDP_Data_Link_Disconnect_Indication_Data_t  HDP_Data_Link_Disconnect_Indication_Data;
      HDP_Data_Link_Data_Indication_Data_t        HDP_Data_Link_Data_Indication_Data;
      HDP_Sync_Capabilities_Indication_t          HDP_Sync_Capabilities_Indication_Data;
      HDP_Sync_Capabilities_Confirmation_t        HDP_Sync_Capabilities_Confirmation_Data;
      HDP_Sync_Set_Indication_t                   HDP_Sync_Set_Indication_Data;
      HDP_Sync_Set_Confirmation_t                 HDP_Sync_Set_Confirmation_Data;
      HDP_Sync_Info_Indication_t                  HDP_Sync_Info_Indication_Data;
   } EventData;
} HDPM_HDP_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the HDPM_NotifyUpdate() function).                           */
typedef struct _tagHDPM_Update_Data_t
{
   HDPM_Update_Type_t UpdateType;
   union
   {
      HDPM_HDP_Event_Data_t HDPEventData;
   } UpdateData;
} HDPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Health Device Manager of a specific Update Event.   */
   /* The Health Device Manager can then take the correct action to     */
   /* process the update.                                               */
Boolean_t HDPM_NotifyUpdate(HDPM_Update_Data_t *UpdateData);

#endif
