/*****< CPPMDEVM.c >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMDEVM - Cycling Power Collector Device Management Header               */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation.                               */
/******************************************************************************/
#ifndef __CPPMDEVMH__
#define __CPPMDEVMH__

   /* CPPM_DeviceManagerHandlerFunction is included in the MODC         */
   /* ModuleHandlerList array. It is the DEVM callback function for the */
   /* module.                                                           */
void BTPSAPI CPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

#endif
