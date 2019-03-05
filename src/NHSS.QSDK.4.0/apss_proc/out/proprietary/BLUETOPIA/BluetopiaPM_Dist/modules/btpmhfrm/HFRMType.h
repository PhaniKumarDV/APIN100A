/*****< hfrmtype.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HFRMTYPE - Hands Free Manager API Type Definitions and Constants for      */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HFRMTYPEH__
#define __HFRMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "HFRMMSG.h"             /* BTPM Hands Free Manager Message Formats.  */

   /* The following enumerated type represents the local role types of  */
   /* Hands Free connections that are supported by the Hands Free       */
   /* Manager.                                                          */
typedef enum
{
   hctHandsFree,
   hctAudioGateway
} HFRM_Connection_Type_t;

   /* The following structure defines the information that an Audio     */
   /* Gateway will respond with pertaining to an individual subscriber  */
   /* information entry when the subscriber list is queried by a Hands  */
   /* Free device.                                                      */
typedef struct _tagHFRM_Subscriber_Number_Information_t
{
   unsigned int   ServiceType;
   unsigned int   NumberFormat;
   char          *PhoneNumber;
} HFRM_Subscriber_Number_Information_t;

   /* The following constants represent the flags that be used with SCO */
   /* Data Indication Events.                                           */
#define HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_MASK                     0xE0000000
#define HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_CORRECTLY_RECEIVED_DATA  0x00000000
#define HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_POSSIBLY_INVALID_DATA    0x10000000
#define HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED         0x20000000
#define HFRM_AUDIO_DATA_FLAGS_PACKET_STATUS_DATA_PARTIALLY_LOST      0x30000000

#endif
