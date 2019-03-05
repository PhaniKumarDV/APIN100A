/*****< btpmhddm.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDDM - HID Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/11/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMHDDMH__
#define __BTPMHDDMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "HDDMAPI.h"             /* HID Manager API Prototypes/Constants.     */

   /* The following type is used with the HDDM_Update_Data_t structure  */
   /* (which is used with the HDDM_NotifyUpdate() function to inform the*/
   /* HID Manager that an Update needs to be dispatched.                */
typedef enum
{
   utHIDEvent
} HDDM_Update_Type_t;

typedef struct _tagHDDM_HID_Event_Data_t
{
   HID_Event_Type_t EventType;
   union
   {
      HID_Open_Request_Indication_Data_t      HID_Open_Request_Indication_Data;
      HID_Open_Indication_Data_t              HID_Open_Indication_Data;
      HID_Open_Confirmation_Data_t            HID_Open_Confirmation_Data;
      HID_Close_Indication_Data_t             HID_Close_Indication_Data;
      HID_Control_Indication_Data_t           HID_Control_Indication_Data;
      HID_Get_Report_Indication_Data_t        HID_Get_Report_Indication_Data;
      HID_Set_Report_Indication_Data_t        HID_Set_Report_Indication_Data;
      HID_Get_Protocol_Indication_Data_t      HID_Get_Protocol_Indication_Data;
      HID_Set_Protocol_Indication_Data_t      HID_Set_Protocol_Indication_Data;
      HID_Get_Idle_Indication_Data_t          HID_Get_Idle_Indication_Data;
      HID_Set_Idle_Indication_Data_t          HID_Set_Idle_Indication_Data;
      HID_Data_Indication_Data_t              HID_Data_Indication_Data;
   } EventData;
} HDDM_HID_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the HDDM_NotifyUpdate() function).                           */
typedef struct _tagHDDM_Update_Data_t
{
   HDDM_Update_Type_t UpdateType;
   union
   {
      HDDM_HID_Event_Data_t HIDEventData;
   } UpdateData;
} HDDM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the HID Manager of a specific Update Event.  The HID    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t HDDM_NotifyUpdate(HDDM_Update_Data_t *UpdateData);

#endif
