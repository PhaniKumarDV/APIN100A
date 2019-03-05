/*****< btpmpanm.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPANM - PAN Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/28/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMPANMH__
#define __BTPMPANMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "PANMAPI.h"             /* PAN Manager API Prototypes/Constants.     */

   /* The following type is used with the PANM_Update_Data_t structure  */
   /* (which is used with the PANM_NotifyUpdate() function to inform the*/
   /* PAN Manager that an Update needs to be dispatched.                */
typedef enum
{
   utPANEvent
} PANM_Update_Type_t;

typedef struct _tagPANM_PAN_Event_Data_t
{
   PAN_Event_Type_t EventType;
   union
   {
      PAN_Open_Request_Indication_Data_t PAN_Open_Request_Indication_Data;
      PAN_Open_Indication_Data_t         PAN_Open_Indication_Data;
      PAN_Open_Confirmation_Data_t       PAN_Open_Confirmation_Data;
      PAN_Close_Indication_Data_t        PAN_Close_Indication_Data;
   } EventData;
} PANM_PAN_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the PANM_NotifyUpdate() function).                           */
typedef struct _tagPANM_Update_Data_t
{
   PANM_Update_Type_t UpdateType;
   union
   {
      PANM_PAN_Event_Data_t PANEventData;
   } UpdateData;
} PANM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the PAN Manager of a specific Update Event. The PAN     */
   /* Manager can then take the correct action to process the update.   */
Boolean_t PANM_NotifyUpdate(PANM_Update_Data_t *UpdateData);

#endif
