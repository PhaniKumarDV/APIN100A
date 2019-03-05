/*****< hogmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HOGMTYPE - HID over GATT Manager API Type Definitions and Constants for   */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HOGMTYPEH__
#define __HOGMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

   /* The following enumerated type represents the valid HID Protocol   */
   /* Modes.                                                            */
typedef enum
{
   hpmBoot,
   hpmReport
} HOGM_HID_Protocol_Mode_t;

   /* The following enumerated type represents the valid HID Report     */
   /* Types.                                                            */
typedef enum
{
   hrtInput,
   hrtOutput,
   hrtFeature,
   hrtBootKeyboardInput,
   hrtBootKeyboardOutput,
   hrtBootMouseInput
} HOGM_HID_Report_Type_t;

   /* The following structure is a container structure which contains   */
   /* all of the information that is needed on a report in order to do a*/
   /* Get/Set procedure.                                                */
typedef struct _tagHOGM_HID_Report_Information_t
{
   HOGM_HID_Report_Type_t ReportType;
   Byte_t                 ReportID;
} HOGM_HID_Report_Information_t;

   /* The following structure is a container structure that contains all*/
   /* of the HID Information that is present in a HID Device.           */
typedef struct _tagHOGM_HID_Information_t
{
   unsigned int HIDVersion;
   unsigned int CountryCode;
   unsigned int VendorID_Source;
   unsigned int VendorID;
   unsigned int ProductID;
   unsigned int ProductVersion;
} HOGM_HID_Information_t;

   /* The following defines all of the valid Vendor ID source values    */
   /* that may be set in the VendorID_Source member of the              */
   /* HOGM_HID_Information_t structure.                                 */
#define HOGM_HID_INFORMATION_VENDOR_ID_SOURCE_BLUETOOTH_SIG        0x00000001
#define HOGM_HID_INFORMATION_VENDOR_ID_SOURCE_USB                  0x00000002

#endif

