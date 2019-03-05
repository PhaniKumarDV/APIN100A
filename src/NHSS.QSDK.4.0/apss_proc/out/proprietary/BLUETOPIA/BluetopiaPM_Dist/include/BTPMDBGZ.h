/*****< btpmdbgz.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMDBGZ - Debug Zone definitions for the Stonestreet One Bluetooth       */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/25/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMDBGZH__
#define __BTPMDBGZH__

   /* Implementation specific Debug Zones.                              */
   /* * NOTE * These cannot clash (or should not overwrite) the         */
   /*          constants that are used by the platform manager debug.   */
   /* * NOTE * All of these Debug Zones are located on Page 0.          */
#define BTPM_DEBUG_ZONE_AUDIO                   0x00100000
#define BTPM_DEBUG_ZONE_HID                     0x00200000
#define BTPM_DEBUG_ZONE_HANDS_FREE              0x00400000
#define BTPM_DEBUG_ZONE_PHONE_BOOK_ACCESS       0x00800000
#define BTPM_DEBUG_ZONE_PAN                     0x01000000
#define BTPM_DEBUG_ZONE_MESSAGE_ACCESS          0x02000000
#define BTPM_DEBUG_ZONE_FTP                     0x04000000
#define BTPM_DEBUG_ZONE_HEADSET                 0x08000000
#define BTPM_DEBUG_ZONE_HEALTH_DEVICE           0x10000000

   /* Debug Zones for Page 1.                                           */
#define BTPM_DEBUG_ZONE_OBJECT_PUSH             0x40000100
#define BTPM_DEBUG_ZONE_ANT_PLUS                0x40000200
#define BTPM_DEBUG_ZONE_HID_DEVICE              0x40000400
#define BTPM_DEBUG_ZONE_3D_SYNC                 0x40000800

   /* Debug Zones for Page 2.                                           */
#define BTPM_DEBUG_ZONE_LE_ALERT_NOTIFICATION   0x80000100
#define BTPM_DEBUG_ZONE_LE_HEART_RATE           0x80000200
#define BTPM_DEBUG_ZONE_LE_PROXIMITY            0x80000400
#define BTPM_DEBUG_ZONE_LE_FIND_ME              0x80000800
#define BTPM_DEBUG_ZONE_LE_TIME                 0x80001000
#define BTPM_DEBUG_ZONE_LE_PHONE_ALERT          0x80002000
#define BTPM_DEBUG_ZONE_LE_BATTERY              0x80004000
#define BTPM_DEBUG_ZONE_LE_BLOOD_PRESSURE       0x80008000
#define BTPM_DEBUG_ZONE_LE_HID_OVER_GATT        0x80010000
#define BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER   0x80020000
#define BTPM_DEBUG_ZONE_LE_GLUCOSE              0x80040000
#define BTPM_DEBUG_ZONE_LE_ANCS                 0x80080000
#define BTPM_DEBUG_ZONE_LE_CYCLING_POWER        0x80100000
#define BTPM_DEBUG_ZONE_LE_CYCLING_SPEED        0x80200000
#define BTPM_DEBUG_ZONE_LE_RUNNING_SPEED        0x80400000

   /* Debug Zones for Page 3.                                           */

#endif
