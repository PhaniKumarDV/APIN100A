/*****< btpmblpm.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMBLPM - BLP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMBLPMH__
#define __BTPMBLPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTGAT.h"            /* Bluetopia GATT Prototypes/Constants.      */
#include "SS1BTBLS.h"            /* Bluetopia BLP Prototypes/Constants.       */

#include "BLPMAPI.h"             /* BLP Manager API Prototypes/Constants.     */

   /* The following type is used with the BLPM_Update_Data_t structure  */
   /* (which is used with the BLPM_NotifyUpdate() function to inform the*/
   /* BLP Manager that an Update needs to be dispatched.                */
typedef enum
{
   utGATTNotificationEvent,
   utGATTIndicationEvent,
   utGATTClientEvent
} BLPM_Update_Type_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the BLPM_NotifyUpdate() function).                           */
typedef struct _tagBLPM_Update_Data_t
{
   BLPM_Update_Type_t UpdateType;
   union
   {
      GATT_Server_Notification_Data_t GATTServerNotificationData;
      GATT_Server_Indication_Data_t   GATTServerIndicationData;
      GATT_Client_Event_Data_t        GATTClientEventData;
   } UpdateData;
} BLPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the BLP Manager of a specific Update Event.  The BLP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t BLPM_NotifyUpdate(BLPM_Update_Data_t *UpdateData);

#endif
