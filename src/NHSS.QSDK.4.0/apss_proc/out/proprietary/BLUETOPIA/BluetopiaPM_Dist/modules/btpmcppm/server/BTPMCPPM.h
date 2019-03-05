/*****< BTPMCPPM.h >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMCPPM - Cycling Power Profile Manager server header file               */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMCPPMH__
#define __BTPMCPPMH__

extern Boolean_t         Initialized;

extern unsigned int      GATMCallbackID;

   /* Pointer to remote device list.                                    */
extern Device_Entry_t   *DeviceEntryList;

   /* Pointer to list of callback registration information entries.     */
extern Callback_Entry_t *CallbackEntryList;

   /* ServerEventCPPM calls the function registered by                  */
   /* CPPM_Register_Collector_Event_Callback for server side            */
   /* applications.                                                     */
void ServerEventCPPM(CPPM_Event_Data_t *EventData);

#endif
