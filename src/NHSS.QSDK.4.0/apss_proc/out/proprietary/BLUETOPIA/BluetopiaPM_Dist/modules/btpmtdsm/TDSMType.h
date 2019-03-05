/*****< tdsmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSMTYPE - 3D Sync Manager API Type Definitions and Constants for         */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __TDSMTYPEH__
#define __TDSMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "TDSMMSG.h"             /* BTPM 3D Sync Manager Message Formats.     */

   /* The following structure is used with                              */
   /* TDSM_Write_Synchronization_Train_Parameters function to specify   */
   /* the train parameters.                                             */
typedef struct _tagTDSM_Synchronization_Train_Parameters_t
{
   Word_t  MinInterval;
   Word_t  MaxInterval;
   DWord_t Timeout;
   Byte_t  ServiceData;
} TDSM_Synchronization_Train_Parameters_t;

   /* The following structure is used with                              */
   /* TDSM_Enable_Connectionless_Slave_Broadcats function to specify the*/
   /* broadcast parameters.                                             */
typedef struct _tagTDSM_Connectionless_Slave_Broadcast_Parameters_t
{
   Word_t    MinInterval;
   Word_t    MaxInterval;
   Word_t    SupervisionTimeout;
   Boolean_t LowPowerEnabled;
} TDSM_Connectionless_Slave_Broadcast_Parameters_t;

#define TDSM_CONNECTIONLESS_SLAVE_BROADCAST_PARAMETERS_SIZE             sizeof(TDSM_Connectionless_Slave_Broadcast_Parameters_t)

   /* The following structure is a container for the information        */
   /* currently being used in the synchonization broadcasts.            */
typedef struct _tagTDSM_Current_Broadcast_Information_t
{
   Boolean_t        CurrentBroadcasting;
   Word_t           LastKnownPeriod;
   Byte_t           LastKnownPeriodFraction;
   TDS_Video_Mode_t VideoMode;
   Byte_t           SyncsPerClockCapture;
   int              LeftLensShutterOpenOffset;
   int              LeftLensShutterCloseOffset;
   int              RightLensShutterOpenOffset;
   int              RightLensShutterCloseOffset;
} TDSM_Current_Broadcast_Information_t;

#define TDSM_CURRENT_BROADCAST_INFORMATION_SIZE                         sizeof(TDSM_Current_Broadcast_Information_t)

   /* The following structure is used with                              */
   /* TDSM_Update_Broadcast_Information() in order to update the        */
   /* information in the synchronization broadcasts. The UpdateFlags    */
   /* member is used to specify which other members are valid and should*/
   /* be updated.  The Broadcast3D member is used to specify whether the*/
   /* CSB data should include 3D frame sync information or broadcast the*/
   /* 2D mode flag. The offset parameters can be positive or negative   */
   /* and indicate an offset to the calculated shutter open and close   */
   /* timing. For instance, by default the left shutter will close at   */
   /* Period/2, but a supplied offset if -5 will change the broadcasted */
   /* close offset to Period/2-5.                                       */
   /* * NOTE * LeftLensShutterOpenOffset must be positive or 0 and the  */
   /*          RightLensShutterCloseOffset must be negative or 0        */
   /*          since they are at the beginning and end of the period    */
   /*          respecitively.                                           */
typedef struct _tagTDSM_Broadcast_Information_Update_t
{
   unsigned long    UpdateFlags;
   TDS_Video_Mode_t VideoMode;
   Byte_t           SyncsPerClockCapture;
   int              LeftLensShutterOpenOffset;
   int              LeftLensShutterCloseOffset;
   int              RightLensShutterOpenOffset;
   int              RightLensShutterCloseOffset;
   Boolean_t        Broadcast3D;
} TDSM_Broadcast_Information_Update_t;

#define TDSM_BRODCAST_INFORMATION_UPDATE_SIZE                           sizeof(TDSM_Broadcast_Information_Update_t)

#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_VIDEO_MODE              0x00000001
#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_SYNCS_PER_CLOCK_CAPTURE 0x00000002
#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_LLS_OPEN_OFFSET         0x00000004
#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_LLS_CLOSE_OFFSET        0x00000008
#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_RLS_OPEN_OFFSET         0x00000010
#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_RLS_CLOSE_OFFSET        0x00000020
#define TDSM_BROADCAST_INFORMATION_UPDATE_FLAGS_BROADCAST_3D            0x00000040


#endif
