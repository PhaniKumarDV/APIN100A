/*****< hidmtype.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDMTYPE - Human Interface Device (HID) Manager API Type Definitions and  */
/*             Constants for Stonestreet One Bluetooth Protocol Stack Platform*/
/*             Manager.                                                       */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/07/14  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HIDMTYPEH__
#define __HIDMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

   /* The following constant represents the value to use for an Invalid */
   /* Report ID when a Report ID is not used.                           */
#define HIDM_INVALID_REPORT_ID                                 (0)

   /* The following enumerated type represents the possible result codes*/
   /* sent in response to HID Transactions.                             */
typedef enum
{
   hmrSuccessful,
   hmrNotReady,
   hmrErrInvalidReportID,
   hmrErrUnsupportedRequest,
   hmrErrInvalidParameter,
   hmrErrUnknown,
   hmrErrFatal,
   hmrData
} HIDM_Result_t;

   /* The following enumerated type represents the available Report     */
   /* Types for GET_REPORT and SET_REPORT transactions.                 */
typedef enum
{
   hmrtInput,
   hmrtOutput,
   hmrtFeature
} HIDM_Report_Type_t;

   /* The following enumerated type represents the available Protocols  */
   /* referenced by the SET_PROTOCOL and GET_PROTOCOL transactions.     */
typedef enum
{
   hmpBoot,
   hmpReport
} HIDM_Protocol_t;

#endif
