/*****< htpmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMTYPE - Health Thermometer Manager API Type Definitions and Constants  */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HTPMTYPEH__
#define __HTPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "HTPMMSG.h"      /* BTPM Health Thermometer Manager Message Formats. */

   /* The following enumerated type represents all of the Connection    */
   /* types that are supported by HTPM.                                 */
typedef enum
{
   httSensor,
   httCollector
} HTPM_Connection_Type_t;

   /* The following enumerated type represents all of the Temperature   */
   /* Measurement Types that are supported by HTPM.                     */
typedef enum
{
   tmtIntermediateMeasurement,
   tmtTemperatureMeasurement
} HTPM_Temperature_Measurement_Type_t;

   /* The following enumerated type represents all of the Temperature   */
   /* Types that are supported by HTPM.                                 */
typedef enum
{
   httArmpit,
   httBody,
   httEar,
   httFinger,
   httGastroIntestinalTract,
   httMouth,
   httRectum,
   httToe,
   httTympanum
} HTPM_Temperature_Type_t;

   /* The following structure defines the format of a HTPM Time Stamp   */
   /* that may be included in Temperature Measurements.                 */
typedef struct _tagHTPM_Time_Stamp_Data_t
{
   unsigned int Year;
   unsigned int Month;
   unsigned int Day;
   unsigned int Hours;
   unsigned int Minutes;
   unsigned int Seconds;
} HTPM_Time_Stamp_Data_t;

#endif

