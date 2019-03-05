/*****< btpmaudm.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMAUDM - Audio Manager for Stonestreet One Bluetooth Protocol Stack     */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */
#include "SS1BTAVC.h"

//todo: remove un-need includes.
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMAUDM.h"            /* BTPM Audio Manager Prototypes/Constants.  */
#include "AUDMMSG.h"             /* BTPM Audio Manager Message Formats.       */
#include "AUDMGR.h"              /* Audio Manager Impl. Prototypes/Constants. */
#include "AUDMUTIL.h"            /* Audio Manager Util. Prototypes/Constants. */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following stub functions are implemented here so that the     */
   /* AVRCP library from Bluetopia can be linked in to the client       */
   /* library of PM, without having to include the libraries these      */
   /* functions are included.                                           */

int BTPSAPI AVCTP_Register_Profile_SDP_Record(unsigned int BluetoothStackID, AVCTP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, char *ProviderName, DWord_t *SDPServiceRecordHandle)
{
   return 0;
}

int BTPSAPI AVCTP_Register_Profile_SDP_Record_Version(unsigned int BluetoothStackID, AVCTP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, char *ProviderName, AVCTP_Version_t AVCTPVersion, DWord_t *SDPServiceRecordHandle)
{
   return 0;
}

int BTPSAPI SDP_Add_Attribute(unsigned int BluetoothStackID, DWord_t Service_Record_Handle, Word_t Attribute_ID, SDP_Data_Element_t *SDP_Data_Element)
{
   return 0;
}

int BTPSAPI SDP_Delete_Attribute(unsigned int BluetoothStackID, DWord_t Service_Record_Handle, Word_t Attribute_ID)
{
   return 0;
}

int BTPSAPI SDP_Delete_Service_Record(unsigned int BluetoothStackID, DWord_t Service_Record_Handle)
{
   return 0;
}
