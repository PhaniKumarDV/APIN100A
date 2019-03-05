/*****< btpmfmpm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMFMPM - FMP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/27/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMFMPMH__
#define __BTPMFMPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTIAS.h"            /* Bluetopia IAS Prototypes/Constants.       */

#include "FMPMAPI.h"             /* FMP Manager API Prototypes/Constants.     */

   /* The following type is used with the FMPM_Update_Data_t structure  */
   /* (which is used with the FMPM_NotifyUpdate() function to inform the*/
   /* FMP Manager that an Update needs to be dispatched.                */
typedef enum
{
   utFMPTargetEvent
} FMPM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all FMPM Target event data.                               */
typedef struct _tagFMPM_Target_Event_Data_t
{
   IAS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      IAS_Alert_Level_Control_Point_Command_Data_t ControlPointCommand;
   } Event_Data;
} FMPM_Target_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the GATM_NotifyUpdate() function).                           */
typedef struct _tagFMPM_Update_Data_t
{
   FMPM_Update_Type_t UpdateType;
   union
   {
      FMPM_Target_Event_Data_t TargetEventData;
   } UpdateData;
} FMPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the FMP Manager of a specific Update Event.  The FMP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t FMPM_NotifyUpdate(FMPM_Update_Data_t *UpdateData);

#endif
