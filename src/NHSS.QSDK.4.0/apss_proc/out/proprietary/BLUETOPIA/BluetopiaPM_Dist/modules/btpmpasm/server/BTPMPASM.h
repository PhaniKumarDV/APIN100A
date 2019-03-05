/*****< btpmpasm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPASM - Phone Alert Status (PAS) Manager for Stonestreet One Bluetooth */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange        Initial creation.                              */
/******************************************************************************/
#ifndef __BTPMPASMH__
#define __BTPMPASMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTPS.h"             /* Bluetopia API Prototypes/Constants.       */
#include "SS1BTPAS.h"            /* PASS Service API Prototypes/Constants.    */

#include "PASMAPI.h"             /* PAS Manager API Prototypes/Constants.     */

   /* The following type is used with the PASM_Update_Data_t structure  */
   /* (which is used with the PASM_NotifyUpdate() function to inform the*/
   /* PAS Manager that an Update needs to be dispatched.                */
typedef enum
{
   utPASSServerEvent
} PASM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all PASM PASS server event data.                          */
typedef struct _tagPASM_PASS_Event_Data_t
{
   PASS_Event_Type_t Event_Type;
   union
   {
      PASS_Read_Client_Configuration_Data_t   PASS_Read_Client_Configuration_Data;
      PASS_Client_Configuration_Update_Data_t PASS_Client_Configuration_Update_Data;
      PASS_Ringer_Control_Command_Data_t      PASS_Ringer_Control_Command_Data;
   } Event_Data;
} PASM_PASS_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the PASM_NotifyUpdate() function).                           */
typedef struct _tagPASM_Update_Data_t
{
   PASM_Update_Type_t UpdateType;
   union
   {
      PASM_PASS_Event_Data_t PASSEventData;
   } UpdateData;
} PASM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the PAS Manager of a specific Update Event.  The PAS    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t PASM_NotifyUpdate(PASM_Update_Data_t *UpdateData);

#endif
