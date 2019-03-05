/*****< hddmtype.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDDMTYPE - Human Interface Device (HID) Device Role Manager API Type      */
/*             Definitions and Constants for Stonestreet One Bluetooth        */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/28/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __HDDMTYPEH__
#define __HDDMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

   /* The following constant represents the value to use for an Invalid */
   /* Report ID when a Report ID is not used.                           */
#define HDDM_INVALID_REPORT_ID                                 (0)

   /* The following enumerated type represents the possible result codes*/
   /* sent in response to HID Transactions.                             */
typedef enum
{
   hdrSuccessful,
   hdrNotReady,
   hdrErrInvalidReportID,
   hdrErrUnsupportedRequest,
   hdrErrInvalidParameter,
   hdrErrUnknown,
   hdrErrFatal,
   hdrData
} HDDM_Result_t;

   /* The following enumerated type represents the available Report     */
   /* Types for GET_REPORT and SET_REPORT transactions.                 */
typedef enum
{
   hdrtInput,
   hdrtOutput,
   hdrtFeature,
   hdrtOther
} HDDM_Report_Type_t;

   /* The following enumerated type represents the available Protocols  */
   /* referenced by the SET_PROTOCOL and GET_PROTOCOL transactions.     */
typedef enum
{
   hdpBoot,
   hdpReport
} HDDM_Protocol_t;


typedef enum
{
   hdcNop,
   hdcHardReset,
   hdcSoftReset,
   hdcSuspend,
   hdcExitSuspend,
   hdcVirtualCableUnplug
} HDDM_Control_Operation_t;

typedef enum
{
   hgrSizeOfReport,
   hgrUseBufferSize
} HDDM_Get_Report_Size_Type_t;

#endif

