/*****< hdsmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDSMTYPE - Headset Manager API Type Definitions and Constants for         */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HDSMTYPEH__
#define __HDSMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "HDSMMSG.h"             /* BTPM Headset Manager Message Formats.     */

   /* The following enumerated type represents the local role types of  */
   /* Headset connections that are supported by the Headset Manager.    */
typedef enum
{
   sctHeadset,
   sctAudioGateway
} HDSM_Connection_Type_t;

   /* The following structure is used with the                          */
   /* HDSM_Query_Current_Configuration() function as a container to hold*/
   /* the currently configured configuration.  See the                  */
   /* HDSM_Query_Current_Configuration() function for more information. */
typedef struct _tagHDSM_Current_Configuration_t
{
   unsigned long IncomingConnectionFlags;
   unsigned long SupportedFeaturesMask;
} HDSM_Current_Configuration_t;

#define HDSM_CURRENT_CONFIGURATION_SIZE                        (sizeof(HDSM_Current_Configuration_t))

   /* The following bit masks define the valid bits that may be set in  */
   /* the SupportedFeaturesMask of the HDSM_Initialization_Data_t       */
   /* structure.  HDSM_SUPPORTED_FEATURES_MASK_HEADSET_XXX bits may be  */
   /* set in the HeadsetInitializationInfo member of the                */
   /* HDSM_Initialization_Info_t structure.                             */
   /* HDSM_SUPPORTED_FEATURES_MASK_AUDIO_GATEWAY_XXX bits may be set in */
   /* the AudioGatewayInitializationInfo member of the                  */
   /* HDSM_Initialization_Info_t structure.                             */
   /* These flags can also be used in their representative structures in*/
   /* the HDSM_Service_Information_t structure.                         */
#define HDSM_SUPPORTED_FEATURES_MASK_AUDIO_GATEWAY_SUPPORTS_IN_BAND_RING            0x00000001
#define HDSM_SUPPORTED_FEATURES_MASK_HEADSET_SUPPORTS_REMOTE_AUDIO_VOLUME_CONTROLS  0x00000001

   /* Structure which holds service data for a HDSET Profile role.      */
typedef struct _tagHDSM_Service_Data_t
{
   unsigned int  PortNumber;
   unsigned long Flags;
} HDSM_Service_Data_t;

   /* Structure which holds service data for both HDSET profile roles.  */
typedef struct _tagHDSM_Service_Information_t
{
   unsigned long       Flags;
   HDSM_Service_Data_t AudioGatewayInformation;
   HDSM_Service_Data_t HeadsetInformation;
} HDSM_Service_Information_t;

   /* The following flags are used with the HDSM_Service_Information_t  */
   /* structure to indicate which Headset roles have been discovered in */
   /* the SDP records and are present in the structure.                 */
#define HDSM_SERVICE_INFORMATION_FLAGS_HEADSET_DATA_VALID                           0x00000001
#define HDSM_SERVICE_INFORMATION_FLAGS_AUDIO_GATEWAY_DATA_VALID                     0x00000002

   /* The following constants represent the flags that be used with SCO */
   /* Data Indication Events.                                           */
#define HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_MASK                                    0xE0000000
#define HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_CORRECTLY_RECEIVED_DATA                 0x00000000
#define HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_POSSIBLY_INVALID_DATA                   0x10000000
#define HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_NO_DATA_RECEIVED                        0x20000000
#define HDSM_AUDIO_DATA_FLAGS_PACKET_STATUS_DATA_PARTIALLY_LOST                     0x30000000

#endif
