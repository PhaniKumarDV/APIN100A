/*****< sdpmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SDPMGR - SDP Interface Implementation for Stonestreet One Bluetooth       */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/02/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __SDPMGRH__
#define __SDPMGRH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "DEVMAPI.h"             /* BTPM Device Manager API Proto./Constants. */

   /* The following function is provided to allow a mechanism for local */
   /* modules to create an SDP Service Record in the Local Devices SDP  */
   /* Database.  This function returns a positive value if successful or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * If this function returns success then the return value   */
   /*          represents the actual Service Record Handle of the       */
   /*          created Service Record.                                  */
BTPSAPI_DECLARATION long BTPSAPI _FTPM_CreateServiceRecord(unsigned int NumberServiceClassUUID, SDP_UUID_Entry_t SDP_UUID_Entry[]);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef long (BTPSAPI *PFN_FTPM_CreateServiceRecord_t)(unsigned int NumberServiceClassUUID, SDP_UUID_Entry_t SDP_UUID_Entry[]);
#endif

#endif
