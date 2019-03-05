/*****< uhid.h >***************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  UHID - Stonestreet One interface for Linux UHID.                          */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/23/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __UHIDH__
#define __UHIDH__

#include "SS1BTPS.h"       /* Platform Manager Prototypes/Constants.          */

#include "SS1BTHIDS.h"     /* HOG Manager Application Programming Interface.  */

#include "UHIDTyp.h"

typedef struct _tagUHID_Output_Report_Event_Data_t
{
	Byte_t Data[UHID_DATA_MAX];
	Word_t Size;
	Byte_t ReportType;
} UHID_Output_Report_Event_Data_t;

typedef struct _tagUHID_Feature_Report_Request_Event_Data_t
{
	DWord_t id;
	Byte_t  ReportNumber;
	Byte_t  ReportType;
} UHID_Feature_Report_Request_Event_Data_t;

   /* The following represents the format of a UHID Event Data.         */
typedef struct _tagUHID_Event_Data_t
{
	UHID_Event_Type_t EventType;
	union
   {
      UHID_Output_Report_Event_Data_t          OutputReportEventData;
      UHID_Feature_Report_Request_Event_Data_t FeatureReportEventData;
	} EventData;
} UHID_Event_Data_t;

   /* The following represents the format of a UHID Event Callback.     */
typedef void (BTPSAPI *UHID_Event_Callback_t)(BD_ADDR_t DeviceAddr, UHID_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is a utility function that is used to      */
   /* initialize the UHID Interface.                                    */
Boolean_t UHID_Initialize(void);

   /* The following function is a utility function that is used to      */
   /* cleanup the UHID Interface.                                       */
Boolean_t UHID_Cleanup(void);

   /* The following function is a utility function that is used to      */
   /* create a UHID device with the specified information.              */
Boolean_t UHID_Create_Device(BD_ADDR_t DeviceAddr, HIDS_HID_Information_Data_t *HIDSInformation, unsigned int ReportDataLength, Byte_t *ReportData, UHID_Event_Callback_t EventCallback, void *CallbackParameter);

   /* The following function is a utility function that is used to      */
   /* destroy a previously created UHID Device.                         */
Boolean_t UHID_Destroy_Device(void);

   /* The following function is a utility function that is used to write*/
   /* a UHID device.                                                    */
Boolean_t UHID_Write_Device(UHID_Command_Data_t *CommandData);

#endif

