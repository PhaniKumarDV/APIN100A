/*****< btpmbasm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMBASM - BAS Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMBASMH__
#define __BTPMBASMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTGAT.h"            /* Bluetopia GATT Prototypes/Constants.      */
#include "SS1BTBAS.h"            /* Bluetopia BAS Prototypes/Constants.       */

#include "BASMAPI.h"             /* BAS Manager API Prototypes/Constants.     */

   /* The following type is used with the BASM_Update_Data_t structure  */
   /* (which is used with the BASM_NotifyUpdate() function to inform the*/
   /* BAS Manager that an Update needs to be dispatched.                */
typedef enum
{
   utGATTNotificationEvent,
   utGATTClientEvent
} BASM_Update_Type_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the BASM_NotifyUpdate() function).                           */
typedef struct _tagBASM_Update_Data_t
{
   BASM_Update_Type_t UpdateType;
   union
   {
      GATT_Server_Notification_Data_t GATTServerNotificationData;
      GATT_Client_Event_Data_t        GATTClientEventData;
   } UpdateData;
} BASM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the BAS Manager of a specific Update Event.  The BAS    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t BASM_NotifyUpdate(BASM_Update_Data_t *UpdateData);

#endif
