/*****< LinuxSPP_SPPLE.h >*****************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LinuxSPP_SPPLE - Bluetooth SPP/SPPLE Emulation using GATT(LE) application.*/
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/16/12  Tim Cook       Initial creation.                               */
/*   02/06/14  Tim Thomas     Ported to Windows.                              */
/******************************************************************************/
#ifndef __LINUXSPP_SSPLEH__
#define __LINUXSPP_SSPLEH__

#include <sys/time.h>
#include "SPPLETyp.h"   /* GATT based SPP-like Types File.                    */
#include "LETPType.h"   /* GATT based Throughput Types File.                  */

#define TimeStamp_t                 struct timeval
#define CAPTURE_TIMESTAMP(_x)       gettimeofday((_x), NULL);
#define DIFF_TIMESTAMP(_x, _y)      (uint32_t)(((((_x).tv_sec & 0xFFFF)*1000)+((_x).tv_usec/1000))-((((_y).tv_sec & 0xFFFF)*1000)+((_y).tv_usec/1000)))

#endif

