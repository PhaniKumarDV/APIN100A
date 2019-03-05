/*****< gatmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GATMTYPE - Generic Attribute Profile Manager API Type Definitions and     */
/*             Constants for Stonestreet One Bluetooth Protocol Stack         */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/16/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GATMTYPEH__
#define __GATMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTGAT.h"            /* Bluetopia GATT API Prototypes/Constants.  */

#include "GATMMSG.h"             /* BTPM GATT Message Formats.                */

   /* The following structure holds the valid information that is stored*/
   /* for a service.                                                    */
typedef struct _tagGATM_Service_Information_t
{
   GATT_UUID_t  ServiceUUID;
   unsigned int ServiceID;
   Word_t       StartHandle;
   Word_t       EndHandle;
} GATM_Service_Information_t;

   /* The following structure holds the valid information that is stored*/
   /* for a GATT Connection.                                            */
typedef struct _tagGATM_Connection_Information_t
{
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} GATM_Connection_Information_t;

#endif
