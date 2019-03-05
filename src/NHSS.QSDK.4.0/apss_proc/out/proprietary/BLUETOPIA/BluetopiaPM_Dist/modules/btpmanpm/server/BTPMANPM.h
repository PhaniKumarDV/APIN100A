/*****< btpmanpm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANPM - ANP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMANPMH__
#define __BTPMANPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTANS.h"            /* Bluetopia ANS Prototypes/Constants.       */

#include "ANPMAPI.h"             /* ANP Manager API Prototypes/Constants.     */

   /* The following type is used with the ANPM_Update_Data_t structure  */
   /* (which is used with the ANPM_NotifyUpdate() function to inform the*/
   /* ANP Manager that an Update needs to be dispatched.                */
typedef enum
{
   utANPEvent,
   utANPConnectionEvent
} ANPM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all ANPM server event data.                               */
typedef struct _tagANPM_Server_Event_Data_t
{
   ANS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      ANS_Read_Client_Configuration_Data_t   ANS_Read_Client_Configuration_Data;
      ANS_Client_Configuration_Update_Data_t ANS_Client_Configuration_Update_Data;
      ANS_Control_Point_Command_Data_t       ANS_Control_Point_Command_Data;
   } Event_Data;
} ANPM_Server_Event_Data_t;

   /* The following structure represents the container structure for    */
   /* holding all GATM connection event data.                           */
typedef struct _tagGATM_Connection_Event_Data_t
{
   GATT_Connection_Event_Type_t Event_Data_Type;
   Word_t                       Event_Data_Size;
   union
   {
      GATT_Device_Connection_Data_t    GATT_Device_Connection_Data;
      GATT_Device_Disconnection_Data_t GATT_Device_Disconnection_Data;
   } Event_Data;
} GATM_Connection_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the ANPM_NotifyUpdate() function).                           */
typedef struct _tagANPM_Update_Data_t
{
   ANPM_Update_Type_t UpdateType;
   union
   {
      ANPM_Server_Event_Data_t     ServerEventData;
      GATM_Connection_Event_Data_t ConnectionEventData;
   } UpdateData;
} ANPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the ANP Manager of a specific Update Event.  The ANP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t ANPM_NotifyUpdate(ANPM_Update_Data_t *UpdateData);

#endif
