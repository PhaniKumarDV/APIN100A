/*****< btpmantm.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMANTM - ANT Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/30/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMANTMH__
#define __BTPMANTMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTANT.h"            /* Bluetopia ANT Prototypes/Constants.       */

#include "ANTMAPI.h"             /* ANT Manager API Prototypes/Constants.     */

   /* The following type is used with the ANTM_Update_Data_t structure  */
   /* (which is used with the ANTM_NotifyUpdate() function to inform the*/
   /* ANT Manager that an Update needs to be dispatched.                */
typedef enum
{
   utANTEvent
} ANTM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all ANTM event data.                                      */
typedef struct _tagANTM_Update_Event_Data_t
{
   ANT_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      ANT_Startup_Message_Event_Data_t                   ANT_Startup_Message_Event_Data;
      ANT_Channel_Response_Event_Data_t                  ANT_Channel_Response_Event_Data;
      ANT_Channel_Status_Event_Data_t                    ANT_Channel_Status_Event_Data;
      ANT_Channel_ID_Event_Data_t                        ANT_Channel_ID_Event_Data;
      ANT_Version_Event_Data_t                           ANT_Version_Event_Data;
      ANT_Capabilities_Event_Data_t                      ANT_Capabilities_Event_Data;
      ANT_Packet_Broadcast_Data_Event_Data_t             ANT_Packet_Broadcast_Data_Event_Data;
      ANT_Packet_Acknowledged_Data_Event_Data_t          ANT_Packet_Acknowledged_Data_Event_Data;
      ANT_Packet_Burst_Data_Event_Data_t                 ANT_Packet_Burst_Data_Event_Data;
      ANT_Packet_Extended_Broadcast_Data_Event_Data_t    ANT_Packet_Extended_Broadcast_Data_Event_Data;
      ANT_Packet_Extended_Acknowledged_Data_Event_Data_t ANT_Packet_Extended_Acknowledged_Data_Event_Data;
      ANT_Packet_Extended_Burst_Data_Event_Data_t        ANT_Packet_Extended_Burst_Data_Event_Data;
      ANT_Raw_Packet_Data_Event_Data_t                   ANT_Raw_Packet_Data_Event_Data;
   } Event_Data;
} ANTM_Update_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the ANTM_NotifyUpdate() function).                           */
typedef struct _tagANTM_Update_Data_t
{
   ANTM_Update_Type_t UpdateType;
   union
   {
      ANTM_Update_Event_Data_t ANTEventData;
   } UpdateData;
} ANTM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the ANT Manager of a specific Update Event.  The ANT    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t ANTM_NotifyUpdate(ANTM_Update_Data_t *UpdateData);

#endif
