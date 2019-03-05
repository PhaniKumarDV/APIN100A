/*****< pasmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMTYPE - Phone Alert Status (PAS) Manager API Type Definitions and      */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __PASMTYPEH__
#define __PASMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "PASMMSG.h"      /* BTPM Phone Alert Manager Message Formats.        */

   /* The following enumerated type is used to indicate the specific    */
   /* type of Phone Alert Status connection.  A Server value indicates  */
   /* the local device is acting as a PAS Server (Role).  A Client value*/
   /* indicates the local device is acting as a PAS Client (Role).      */
typedef enum
{
   pctPASServer,
   pctPASClient
} PASM_Connection_Type_t;

   /* The following structure defines the format of the Alert Status    */
   /* information.                                                      */
typedef struct _tagPASM_Alert_Status_t
{
   Boolean_t RingerStateActive;
   Boolean_t VibrateStateActive;
   Boolean_t DisplayStateActive;
} PASM_Alert_Status_t;

#define PASM_ALERT_STATUS_SIZE                                 (sizeof(PASM_Alert_Status_t))

   /* The following type defines the valid Ringer Setting values.       */
typedef enum
{
   prsSilent,
   prsNormal
} PASM_Ringer_Setting_t;

   /* The following type defines the ringer commands that may be        */
   /* received via the ringer control point.                            */
typedef enum
{
   prcSilent,
   prcMuteOnce,
   prcCancelSilent
} PASM_Ringer_Control_Command_t;

#endif

