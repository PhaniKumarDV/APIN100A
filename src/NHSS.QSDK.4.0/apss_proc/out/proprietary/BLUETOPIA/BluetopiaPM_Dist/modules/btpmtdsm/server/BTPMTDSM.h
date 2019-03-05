/*****< btpmtdsm.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMTDSM - 3D Sync Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMTDSMH__
#define __BTPMTDSMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "TDSMAPI.h"             /* TDS Manager API Prototypes/Constants.     */

   /* The following type is used with the TDSM_Update_Data_t structure  */
   /* (which is used with the TDSM_NotifyUpdate() function to inform the*/
   /* 3D Sync Manager that an Update needs to be dispatched.            */
typedef enum
{
   utTDSEvent
} TDSM_Update_Type_t;

typedef struct _tagTDSM_Headset_Event_Data_t
{
   TDS_Event_Type_t EventType;
   union
   {
      TDS_Display_Connection_Announcement_Data_t        TDS_Display_Connection_Announcement_Data;
      TDS_Display_Synchronization_Train_Complete_Data_t TDS_Display_Synchronization_Train_Complete_Data;
      TDS_Display_Channel_Map_Change_Data_t             TDS_Display_Channel_Map_Change_Data;
   } EventData;
} TDSM_TDS_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the TDSM_NotifyUpdate() function).                           */
typedef struct _tagTDSM_Update_Data_t
{
   TDSM_Update_Type_t UpdateType;
   union
   {
      TDSM_TDS_Event_Data_t TDSEventData;
   } UpdateData;
} TDSM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the 3D Sync Manager of a specific Update Event.  The    */
   /* 3D Sync Manager can then take the correct action to process the   */
   /* update.                                                           */
Boolean_t TDSM_NotifyUpdate(TDSM_Update_Data_t *UpdateData);

#endif
