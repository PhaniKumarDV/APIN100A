/*****< uhidtyp.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  UHIDTYP - Stonestreet One interface for Linux UHID types file.            */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/23/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __UHIDTYPH__
#define __UHIDTYPH__

#include "BTAPITyp.h"
#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following defines the maximum UHID Data Size.                       */
#define UHID_DATA_MAX                                    4096

   /* The following defines the different UHID Bus Types.               */
#define UHID_BUS_TYPE_BLUETOOTH                          0x0005

   /* The following represent all of the valid UHID command/event types.*/
typedef enum
{
   UHID_CREATE,
   UHID_DESTROY,
   UHID_START,
   UHID_STOP,
   UHID_OPEN,
   UHID_CLOSE,
   UHID_OUTPUT,
   UHID_OUTPUT_EV,
   UHID_INPUT,
   UHID_FEATURE,
   UHID_FEATURE_ANSWER,
} UHID_Event_Type_t;

   /* The following represents all of the valid UHID Report Types.      */
typedef enum
{
	UHID_FEATURE_REPORT,
	UHID_OUTPUT_REPORT,
	UHID_INPUT_REPORT
} UHID_Report_Type_t;

   /* The following represents the UHID Command Data that is sent to    */
   /* UHID when creating a UHID Device.                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Create_Request_t
{
	Byte_t   DeviceName[128];
	Byte_t   Phys[64];
	Byte_t   Uniq[64];
	Byte_t  *ReportDescriptor;
	Word_t   ReportDescriptorLength;
	Word_t   BusType;
	DWord_t  VendorID;
	DWord_t  ProductID;
	DWord_t  ProductVersion;
	DWord_t  CountryCode;
} __PACKED_STRUCT_END__ UHID_Create_Request_t;

   /* The following represent the UHID Report Data that is sent/received*/
   /* from UHID.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Input_Report_t
{
	Byte_t Data[UHID_DATA_MAX];
	Word_t Size;
} __PACKED_STRUCT_END__ UHID_Input_Report_t;

typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Output_Report_t
{
	Byte_t Data[UHID_DATA_MAX];
	Word_t Size;
	Byte_t ReportType;
} __PACKED_STRUCT_END__ UHID_Output_Report_t;

typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Output_Report_Ev_t
{
	Word_t   Type;
	Word_t   Code;
	SDWord_t Value;
} __PACKED_STRUCT_END__  UHID_Output_Report_Ev_t;

typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Feature_Report_Request_t
{
	DWord_t id;
	Byte_t  ReportNumber;
	Byte_t  ReportType;
} __PACKED_STRUCT_END__ UHID_Feature_Report_Request_t;

typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Feature_Report_Answer_t
{
	DWord_t id;
	Word_t  Error;
	Word_t  DataSize;
	Byte_t  Data[UHID_DATA_MAX];
} __PACKED_STRUCT_END__ UHID_Feature_Report_Answer_t;

typedef __PACKED_STRUCT_BEGIN__ struct _tagUHID_Command_Data_t
{
	DWord_t EventType;
	union
   {
      UHID_Create_Request_t         CreateRequestData;
      UHID_Input_Report_t           InputReportData;
      UHID_Output_Report_t          OutputReportData;
      UHID_Output_Report_Ev_t       OutputReportEvData;
      UHID_Feature_Report_Request_t FeatureReportRequestData;
      UHID_Feature_Report_Answer_t  FeatureReportAnswerData;
	} EventData;
} __PACKED_STRUCT_END__ UHID_Command_Data_t;

#endif

