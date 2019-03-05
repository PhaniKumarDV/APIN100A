/*****< btpmhidm.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHIDM - HID Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/18/11  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMHIDMH__
#define __BTPMHIDMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "HIDMAPI.h"             /* HID Manager API Prototypes/Constants.     */

   /* The following type is used with the HIDM_Update_Data_t structure  */
   /* (which is used with the HIDM_NotifyUpdate() function to inform the*/
   /* HID Manager that an Update needs to be dispatched.                */
typedef enum
{
   utHIDHEvent
} HIDM_Update_Type_t;

typedef struct _tagHIDM_HIDH_Event_Data_t
{
   HID_Host_Event_Type_t EventType;
   union
   {
      HID_Host_Open_Request_Indication_Data_t   HID_Host_Open_Request_Indication_Data;
      HID_Host_Open_Indication_Data_t           HID_Host_Open_Indication_Data;
      HID_Host_Open_Confirmation_Data_t         HID_Host_Open_Confirmation_Data;
      HID_Host_Close_Indication_Data_t          HID_Host_Close_Indication_Data;
      HID_Host_Boot_Keyboard_Data_t             HID_Host_Boot_Keyboard_Data;
      HID_Host_Data_Indication_Data_t           HID_Host_Data_Indication_Data;
      HID_Host_Boot_Keyboard_Repeat_Data_t      HID_Host_Boot_Keyboard_Repeat_Data;
      HID_Host_Boot_Mouse_Data_t                HID_Host_Boot_Mouse_Data;
      HID_Host_Get_Report_Confirmation_Data_t   HID_Host_Get_Report_Confirmation_Data;
      HID_Host_Set_Report_Confirmation_Data_t   HID_Host_Set_Report_Confirmation_Data;
      HID_Host_Get_Protocol_Confirmation_Data_t HID_Host_Get_Protocol_Confirmation_Data;
      HID_Host_Set_Protocol_Confirmation_Data_t HID_Host_Set_Protocol_Confirmation_Data;
      HID_Host_Get_Idle_Confirmation_Data_t     HID_Host_Get_Idle_Confirmation_Data;
      HID_Host_Set_Idle_Confirmation_Data_t     HID_Host_Set_Idle_Confirmation_Data;
   } EventData;
} HIDM_HIDH_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the HIDM_NotifyUpdate() function).                           */
typedef struct _tagHIDM_Update_Data_t
{
   HIDM_Update_Type_t UpdateType;
   union
   {
      HIDM_HIDH_Event_Data_t HIDHEventData;
   } UpdateData;
} HIDM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the HID Manager of a specific Update Event.  The HID    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t HIDM_NotifyUpdate(HIDM_Update_Data_t *UpdateData);

#endif
