/*****< btpmtipm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMTIPM - TIP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMTIPMH__
#define __BTPMTIPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTCTS.h"            /* Bluetopia CTS Prototypes/Constants.       */
#include "SS1BTRTU.h"            /* Bluetopia RTUS Prototypes/Constants.      */
#include "SS1BTNDC.h"            /* Bluetopia NDCS Prototypes/Constants.      */

#include "TIPMAPI.h"             /* TIP Manager API Prototypes/Constants.     */

   /* The following type is used with the TIPM_Update_Data_t structure  */
   /* (which is used with the TIPM_NotifyUpdate() function to inform the*/
   /* TIP Manager that an Update needs to be dispatched.                */
typedef enum
{
   utCTSServerEvent,
   utGATTClientEvent,
   utGATTConnectionEvent
} TIPM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all TIPM server event data.                               */
typedef struct _tagCTS_Server_Event_Data_t
{
   CTS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      CTS_Read_Client_Configuration_Data_t               CTS_Read_Client_Configuration_Data;
      CTS_Client_Configuration_Update_Data_t             CTS_Client_Configuration_Update_Data;
      CTS_Read_Current_Time_Request_Data_t               CTS_Read_Current_Time_Request_Data;
      CTS_Read_Reference_Time_Information_Request_Data_t CTS_Read_Reference_Time_Information_Request_Data;
   } Event_Data;
} CTS_Server_Event_Data_t;

   /* The following structure represents the container structure for    */
   /* holding all of the GATT Client event data updates.                */
typedef struct _tagTIPM_GATT_Client_Event_Data_t
{
   GATT_Client_Event_Type_t Event_Data_Type;
   Word_t                   Event_Data_Size;
   union
   {
      GATT_Read_Response_Data_t  GATT_Read_Response_Data;
      GATT_Write_Response_Data_t GATT_Write_Response_Data;
      GATT_Request_Error_Data_t  GATT_Request_Error_Data;
   } Event_Data;
} TIPM_GATT_Client_Event_Data_t;

   /* The following structure represents the conatiner structure for    */
   /* holding all of the GATT Connection event data updates.            */
typedef struct _tagTIPM_GATT_Connection_Event_Data_t
{
   GATT_Connection_Event_Type_t Event_Data_Type;
   Word_t                       Event_Data_Size;
   union
   {
      GATT_Server_Notification_Data_t GATT_Server_Notification_Data;
   } Event_Data;
} TIPM_GATT_Connection_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the GATM_NotifyUpdate() function).                           */
typedef struct _tagTIPM_Update_Data_t
{
   TIPM_Update_Type_t UpdateType;
   union
   {
      CTS_Server_Event_Data_t           CTSServerEventData;
      TIPM_GATT_Client_Event_Data_t     GATTClientEventData;
      TIPM_GATT_Connection_Event_Data_t GATTConnectionEventData;
   } UpdateData;
} TIPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the TIP Manager of a specific Update Event.  The TIP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t TIPM_NotifyUpdate(TIPM_Update_Data_t *UpdateData);

#endif
