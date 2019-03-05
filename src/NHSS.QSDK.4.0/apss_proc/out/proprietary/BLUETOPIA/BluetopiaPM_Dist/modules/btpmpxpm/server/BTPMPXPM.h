/*****< btpmpxpm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPXPM - PXP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMPXPMH__
#define __BTPMPXPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "PXPMAPI.h"             /* PXP Manager API Prototypes/Constants.     */

   /* The following type is used with the PXPM_Update_Data_t structure  */
   /* (which is used with the PXPM_NotifyUpdate() function to inform the*/
   /* PXP Manager that an Update needs to be dispatched.                */
typedef enum
{
   utGATTClientEvent
} PXPM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all GATM Client event data.                               */
typedef struct _tagGATM_Client_Event_Data_t
{
   GATT_Client_Event_Type_t Event_Data_Type;
   Word_t                   Event_Data_Size;
   union
   {
      GATT_Request_Error_Data_t  GATT_Request_Error_Data;
      GATT_Read_Response_Data_t  GATT_Read_Response_Data;
      GATT_Write_Response_Data_t GATT_Write_Response_Data;
   } Event_Data;
} GATM_Client_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the GATM_NotifyUpdate() function).                           */
typedef struct _tagPXPM_Update_Data_t
{
   PXPM_Update_Type_t UpdateType;
   union
   {
      GATM_Client_Event_Data_t ClientEventData;
   } UpdateData;
} PXPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the PXP Manager of a specific Update Event.  The PXP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t PXPM_NotifyUpdate(PXPM_Update_Data_t *UpdateData);

#endif
