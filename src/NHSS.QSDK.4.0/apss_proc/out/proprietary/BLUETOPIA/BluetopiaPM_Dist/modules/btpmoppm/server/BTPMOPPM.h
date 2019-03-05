/*****< btpmoppm.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMOPPM - Object Push Manager for Stonestreet One Bluetooth              */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/10/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMOPPMH__
#define __BTPMOPPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "OPPMAPI.h"             /* OPPM Manager API Prototypes/Constants.    */

   /* The following type is used with the OPPM_Update_Data_t structure  */
   /* (which is used with the OPPM_NotifyUpdate() function to inform the*/
   /* Object Push Manager that an Update needs to be dispatched.        */
typedef enum
{
   utOPPEvent
} OPPM_Update_Type_t;

typedef struct _tagOPPM_OPP_Event_Data_t
{
   OPP_Event_Type_t EventType;
   union
   {
      OPP_Open_Request_Indication_Data_t         OpenRequestIndicationData;
      OPP_Open_Port_Indication_Data_t            OpenPortIndicationData;
      OPP_Open_Port_Confirmation_Data_t          OpenPortConfirmationData;
      OPP_Close_Port_Indication_Data_t           ClosePortIndicationData;
      OPP_Abort_Indication_Data_t                AbortIndicationData;
      OPP_Abort_Confirmation_Data_t              AbortConfirmationData;
      OPP_Push_Object_Indication_Data_t          PushObjectIndicationData;
      OPP_Push_Object_Confirmation_Data_t        PushObjectConfirmationData;
      OPP_Pull_Business_Card_Indication_Data_t   PullBusinessCardIndicationData;
      OPP_Pull_Business_Card_Confirmation_Data_t PullBusinessCardConfirmationData;
   } EventData;
} OPPM_OPP_Event_Data_t;

   /* information about what type of update needs to be dispatched (used*/
   /* with the OPPM_NotifyUpdate() function).                           */
typedef struct _tagOPPM_UpdateData_t
{
   OPPM_Update_Type_t UpdateType;
   union
   {
      OPPM_OPP_Event_Data_t OPPEventData;
   } UpdateData;
} OPPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Object Push Manager of a specific Update Event.  The*/
   /* Object Push Manager can then take the correct action to process   */
   /* the update.                                                       */
Boolean_t OPPM_NotifyUpdate(OPPM_Update_Data_t *UpdateData);

#endif
