/*****< btpmmodc.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMOD - Installable Module Handler for Stonestreet One Bluetooth        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/12/10  D. Lange       Initial creation.                               */
/******************************************************************************/

#include "SS1BTPM.h"             /* BTPM API Prototypes and Constants.        */

#include "BTPMMODC.h"            /* BTPM Module Handler List.                 */


   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

#ifdef MODC_SUPPORT_ANCM
#include "SS1BTANCM.h"           /* Apple Notification Center Service Module. */
#endif

#ifdef MODC_SUPPORT_ANPM
#include "SS1BTANPM.h"           /* Alert Notification Manager Module.        */

   /* ANP Manager container initialization.                             */
static ANPM_Initialization_Info_t ANPMInitializationInfo =
{
   ANPM_ALERT_CATEGORY_BIT_MASK_ALL_CATEGORIES,
   ANPM_ALERT_CATEGORY_BIT_MASK_ALL_CATEGORIES
} ;
#endif

#ifdef MODC_SUPPORT_ANTM
#include "SS1BTANTM.h"           /* ANT+ Manager Module.                      */

   /* ANT+ Manager initialization.                                      */
static ANTM_Initialization_Info_t ANTMInitializationInfo =
{
   0
} ;
#endif

#ifdef MODC_SUPPORT_AUDM
#include "SS1BTAUDM.h"           /* Audio Manager Module.                     */

   /* Audio Manager default initialization.                             */
   /*   - A2DP SRC Support.                                             */
static AUD_Stream_Initialization_Info_t SRCInitializationInfo =
{
   0,
   "A2DP Source",
   4,
   {
      /* The supported SBC Stream Formats.                              */
      { 44100, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
      { 48000, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
      { 44100, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
      { 48000, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC }
   },
   0
} ;

   /* Audio Manager default initialization.                             */
   /*   - A2DP SNK Support.                                             */
static AUD_Stream_Initialization_Info_t SNKInitializationInfo =
{
   0,
   "A2DP Sink",
   4,
   {
      /* The supported SBC Stream Formats.                              */
      { 44100, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
      { 48000, 2, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
      { 44100, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC },
      { 48000, 1, AUD_STREAM_FORMAT_FLAGS_CODEC_TYPE_SBC }
   },
   0
} ;

   /* Specify the AVRCP Controller Role Information.                    */
static AUD_Remote_Control_Role_Info_t ControllerRemoteControlRoleInfo =
{
   SDP_AVRCP_SUPPORTED_FEATURES_CONTROLLER_CATEGORY_1,
   "Stonestreet One",
   "AVRCP Controller"
} ;

   /* Specify the AVRCP Target Role Information.                        */
static AUD_Remote_Control_Role_Info_t TargetRemoteControlRoleInfo =
{
   SDP_AVRCP_SUPPORTED_FEATURES_TARGET_CATEGORY_1,
   "Stonestreet One",
   "AVRCP Target"
} ;

   /* Specify the AVRCP Initialization Information.                     */
   /* Specify support for A2DP SRC, A2DP SNK, AVRCP 1.0 CT, and         */
   /* AVRCP 1.0 TG.                                                     */
static AUD_Remote_Control_Initialization_Info_t RemoteControlInitializationInfo =
{
   0,
   apvVersion1_0,
   &ControllerRemoteControlRoleInfo,
   &TargetRemoteControlRoleInfo
} ;

   /* Audio Manager container initialization.                           */
static AUDM_Initialization_Data_t AudioInitializationInfo =
{
   AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION,
   0,
   &SRCInitializationInfo,
   &SNKInitializationInfo,
   &RemoteControlInitializationInfo
} ;
#endif

#ifdef MODC_SUPPORT_BASM
#include "SS1BTBASM.h"           /* Battery Service Manager Module.           */
#endif

#ifdef MODC_SUPPORT_BLPM
#include "SS1BTBLPM.h"           /* Blood Pressure Profile Manager Module.    */
#endif

#ifdef MODC_SUPPORT_CPPM
#include "SS1BTCPPM.h"           /* Cycling Power Profile Manager Module      */
#endif

#ifdef MODC_SUPPORT_CSCM
#include "SS1BTCSCM.h"           /* Cycling Speed and Cadence Manager Module. */
#endif

#ifdef MODC_SUPPORT_FMPM
#include "SS1BTFMPM.h"           /* Find Me Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_FTPM
#include "SS1BTFTPM.h"           /* File Transfer Manager Module.             */
#endif

#ifdef MODC_SUPPORT_GLPM
#include "SS1BTGLPM.h"           /* Glucose Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_HDDM
#include "SS1BTHDDM.h"           /* HID Device Manager Module.                */

   /* The following table represent the Mouse Report Descriptor for this*/
   /* HID Mouse Device.                                                 */
static Byte_t HIDMouseReportDescriptor[] =
{
   0x05, 0x01, /* Usage Page (Generic Desktop)      */
   0x09, 0x02, /* Usage (Mouse)                     */
   0xA1, 0x01, /* Collection Application            */
   0x09, 0x01, /* Usage (Pointer)                   */
   0xA1, 0x00, /* Collection Physical               */
   0X05, 0X09, /* Usage Page (Buttons)              */
   0x19, 0x01, /* Usage Minimum (1)                 */
   0x29, 0x02, /* Usage Maximum (2)                 */
   0x15, 0x00, /* Logical Minimum (0)               */
   0x25, 0x01, /* Logical Maximum (1)               */
   0x95, 0x02, /* Report Count (2)                  */
   0x75, 0x01, /* Report Size (1)                   */
   0x81, 0x02, /* Input (Data, Variable, Absolute)  */
   0x95, 0x01, /* Report Count (1)                  */
   0x75, 0x06, /* Report Size (6)                   */
   0x81, 0x01, /* Input (Constant)                  */
   0x05, 0x01, /* Usage Page (Generic Desktop)      */
   0x09, 0x30, /* Usage (X)                         */
   0x09, 0x31, /* Usage (Y)                         */
   0x15, 0x81, /* Logical Minimum (-127)            */
   0x25, 0x7F, /* Logical Maximum (128)             */
   0x75, 0x08, /* Report Size (8)                   */
   0x95, 0x02, /* Report Count (2)                  */
   0x81, 0x06, /* Input (Data, Variable, Relative)  */
   0xC0,       /* End Collection                    */
   0xC0        /* End Collection                    */
} ;

   /* Container for the HIS Mouse Report Descriptor.                    */
static HID_Descriptor_t HIDMouseDescriptor =
{
   0x22,
   sizeof(HIDMouseReportDescriptor),
   HIDMouseReportDescriptor
} ;

   /* HID Device default initialization.                                */
   /* * NOTE * The default device configuration is for a HID Mouse.     */
static HDDM_Initialization_Data_t HDDMInitializationInfo =
{
   (HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION | HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION),
   0x0100,
   (HID_NORMALLY_CONNECTABLE_BIT | HID_BATTERY_POWER_BIT),
   0x0111,
   0x80,
   1,
   &HIDMouseDescriptor,
   "Stonestreet One Bluetooth Mouse"
} ;
#endif

#ifdef MODC_SUPPORT_HDPM
#include "SS1BTHDPM.h"           /* Health Device Manager Module.             */

   /* Health Device Profile default initialization.                     */
static HDPM_Initialization_Info_t HDPMInitializationInfo =
{
   "Health Device Service",
   "Stonestreet One"
} ;
#endif

#ifdef MODC_SUPPORT_HDSM
#include "SS1BTHDSM.h"           /* Headset Manager Module.                   */

   /* Headset Manager default initialization.                           */
   /*   - Audio Gateway Role Support.                                   */
static HDSM_Initialization_Data_t HDSMInitializationData_AG =
{
   "Headset - AG",
   4,
   HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION,
   0,
   7
};

   /* Headset Manager default initialization.                           */
   /*   - Headset Role Support.                                         */
static HDSM_Initialization_Data_t HDSMInitializationData_Headset =
{
   "Headset",
   2,
   HDSM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION,
   HDSM_SUPPORTED_FEATURES_MASK_HEADSET_SUPPORTS_REMOTE_AUDIO_VOLUME_CONTROLS,
   7
};

   /* Head Set Manager container initialization.                        */
static HDSM_Initialization_Info_t HDSMInitializationInfo =
{
   &HDSMInitializationData_AG,
   &HDSMInitializationData_Headset
} ;
#endif

#ifdef MODC_SUPPORT_HFRM
#include "SS1BTHFRM.h"           /* Hands Free Manager Module.                */

   /* Hands Free Manager default initialization.                        */
   /*   - Audio Gateway Role Support.                                   */
static HFRM_Initialization_Data_t HFRMInitializationData_AG =
{
   "Hands Free Audio Gateway",
   3,
   (HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION | HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION),
   (HFRE_THREE_WAY_CALLING_SUPPORTED_BIT | HFRE_AG_SOUND_ENHANCEMENT_SUPPORTED_BIT | HFRE_REJECT_CALL_SUPPORT_BIT | HFRE_AG_ENHANCED_CALL_STATUS_SUPPORTED_BIT | HFRE_AG_EXTENDED_ERROR_RESULT_CODES_SUPPORTED_BIT | HFRE_AG_VOICE_RECOGNITION_SUPPORTED_BIT),
   (HFRE_RELEASE_ALL_HELD_CALLS | HFRE_RELEASE_ALL_ACTIVE_CALLS_ACCEPT_WAITING_CALL | HFRE_PLACE_ALL_ACTIVE_CALLS_ON_HOLD_ACCEPT_THE_OTHER | HFRE_ADD_A_HELD_CALL_TO_CONVERSATION | HFRE_CONNECT_TWO_CALLS_DISCONNECT_SUBSCRIBER),
   HFRE_NETWORK_TYPE_ABILITY_TO_REJECT_CALLS,
   0,
   NULL,
   7 /* Number of servers. */
} ;

   /* Hands Free Manager default initialization.                        */
   /*   - Hands Free Role Support.                                      */
static HFRM_Initialization_Data_t HFRMInitializationData_HF =
{
   "Hands Free",
   1,
   HFRM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION,
   (HFRE_HF_SOUND_ENHANCEMENT_SUPPORTED_BIT | HFRE_CALL_WAITING_THREE_WAY_CALLING_SUPPORTED_BIT | HFRE_CLI_SUPPORTED_BIT | HFRE_HF_VOICE_RECOGNITION_SUPPORTED_BIT | HFRE_REMOTE_VOLUME_CONTROL_SUPPORTED_BIT | HFRE_HF_ENHANCED_CALL_STATUS_SUPPORTED_BIT | HFRE_HF_ENHANCED_CALL_CONTROL_SUPPORTED_BIT),
   (HFRE_RELEASE_ALL_ACTIVE_CALLS_ACCEPT_WAITING_CALL | HFRE_PLACE_ALL_ACTIVE_CALLS_ON_HOLD_ACCEPT_THE_OTHER),
   0,
   0,
   NULL,
   7 /* Number of servers. */
} ;

   /* Hands Free Manager container initialization.                      */
static HFRM_Initialization_Info_t HFRMInitializationInfo =
{
   &HFRMInitializationData_AG,
   &HFRMInitializationData_HF
} ;
#endif

#ifdef MODC_SUPPORT_HIDM
#include "SS1BTHIDM.h"           /* HID Host Manager Module.                  */
#endif

#ifdef MODC_SUPPORT_HOGM
#include "SS1BTHOGM.h"           /* HID over GATT Manager Module.             */

   /* HOG Manager container initialization.                             */
static HOGM_Initialization_Data_t HOGMInitializationInfo =
{
   HOGM_SUPPORTED_FEATURES_FLAGS_REPORT_MODE
} ;
#endif

#ifdef MODC_SUPPORT_HRPM
#include "SS1BTHRPM.h"           /* Heart Rate Manager Module.                */
#endif

#ifdef MODC_SUPPORT_HTPM
#include "SS1BTHTPM.h"           /* Health Thermometer Manager Module.        */
#endif

#ifdef MODC_SUPPORT_MAPM
#include "SS1BTMAPM.h"           /* Message Access Manager Module.            */

   /* Message Access Profile (MAP) default initialization.              */
static MAPM_Initialization_Data_t MAPMInitializationData =
{
   0,
   "MAP Notification Server"
} ;
#endif

#ifdef MODC_SUPPORT_OPPM
#include "SS1BTOPPM.h"           /* Object Push Manager Module.               */
#endif

#ifdef MODC_SUPPORT_PANM
#include "SS1BTPANM.h"           /* PAN Manager Module.                       */

   /* PAN Manager default initialization.                               */
   /*    - PAN User Service (naming data).                              */
static PANM_Naming_Data_t NamingData_User =
{
   "PAN-U Service",
   "PAN User"
} ;

   /* PAN Manager default initialization.                               */
   /*    - PAN Group Ad-Hoc Service (naming data).                      */
static PANM_Naming_Data_t NamingData_GroupAdHoc =
{
   "PAN-GRP Service",
   "PAN Group Ad-Hoc"
} ;

   /* PAN Manager default initialization.                               */
   /*    - PAN Network Access Point Service (naming data).              */
static PANM_Naming_Data_t NamingData_AccessPoint =
{
   "PAN-NAP Service",
   "PAN Network Access Point"
} ;

   /* PAN Manager default initialization.                               */
   /*    - Network Packet Type List (IPV4/ARP).                         */
static Word_t NetworkPacketTypeList[] =
{
   0x0800,
   0x0806
} ;

   /* PAN Manager container initialization.                             */
static PANM_Initialization_Info_t PANMInitializationInfo =
{
   (PAN_PERSONAL_AREA_NETWORK_USER_SERVICE | PAN_GROUP_ADHOC_NETWORK_SERVICE | PAN_NETWORK_ACCESS_POINT_SERVICE),
   0,
   &NamingData_User,
   &NamingData_AccessPoint,
   &NamingData_GroupAdHoc,
   1,
   NetworkPacketTypeList,
   PAN_SECURITY_DESCRIPTION_SERVICE_LEVEL_ENFORCED_SECURITY,
   PAN_NETWORK_ACCESS_TYPE_GSM,
   0,
   (PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION | PANM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
} ;
#endif

#ifdef MODC_SUPPORT_PASM
#include "SS1BTPASM.h"           /* Phone Alert Status Manager Module.        */
#endif

#ifdef MODC_SUPPORT_PBAM
#include "SS1BTPBAM.h"           /* Phone Book Access Host Manager Module.    */

   /* PBAP Client Manager default initialization.                       */
static PBAM_Client_Initialization_Data_t PBAMClientInitializationData =
{
   "Phonebook Access Client"
} ;

   /* PBAP manager container initialization.                            */
static PBAM_Initialization_Info_t PBAMInitializationInfo =
{
   &PBAMClientInitializationData
} ;
#endif

#ifdef MODC_SUPPORT_PXPM
#include "SS1BTPXPM.h"           /* Proximity Manager Module.                 */

   /* PXP Manager container initialization.                             */
static PXPM_Initialization_Info_t PXPMInitializationInfo =
{
   PXPM_CONFIGURATION_DEFAULT_REFRESH_TIME,
   PXPM_CONFIGURATION_DEFAULT_ALERT_LEVEL,
   PXPM_CONFIGURATION_DEFAULT_PATH_LOSS_THRESHOLD
} ;
#endif

#ifdef MODC_SUPPORT_RSCM
#include "SS1BTRSCM.h"           /* Running Speed and Cadence Manager Module. */
#endif

#ifdef MODC_SUPPORT_TDSM
#include "SS1BTTDSM.h"           /* 3D Sync Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_TIPM
#include "SS1BTTIPM.h"           /* Time Manager Module.                      */

   /* TIP Manager container initialization.                             */
static TIPM_Initialization_Info_t TIPMInitializationInfo =
{
   (TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_CLIENT | TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_SERVER)
} ;
#endif

   /* Main Module list that contains all configured modules.            */
   /* * NOTE * HID Device support is left out as both Host and Device   */
   /*          require the same psm.                                    */
static MOD_ModuleHandlerEntry_t ModuleHandlerList[] =
{
#ifdef MODC_SUPPORT_ANCM
   { ANCM_InitializationHandlerFunction, NULL,                               ANCM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_ANTM
   { ANTM_InitializationHandlerFunction, (void *)(&ANTMInitializationInfo),  ANTM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_ANPM
   { ANPM_InitializationHandlerFunction, (void *)(&ANPMInitializationInfo),  ANPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_AUDM
   { AUDM_InitializationHandlerFunction, (void *)(&AudioInitializationInfo), AUDM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_BASM
   { BASM_InitializationHandlerFunction, NULL,                               BASM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_BLPM
   { BLPM_InitializationHandlerFunction, NULL,                               BLPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_CPPM
   { CPPM_InitializationHandlerFunction, NULL,                               CPPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_CSCM
   { CSCM_InitializationHandlerFunction, NULL,                               CSCM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_FMPM
   { FMPM_InitializationHandlerFunction, NULL,                               FMPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_FTPM
   { FTPM_InitializationHandlerFunction, NULL,                               FTPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_GLPM
   { GLPM_InitializationHandlerFunction, NULL,                               GLPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HDDM
//   { HDDM_InitializationHandlerFunction, (void *)(&HDDMInitializationInfo),  HDDM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HDPM
   { HDPM_InitializationHandlerFunction, (void *)(&HDPMInitializationInfo),  HDPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HDSM
   { HDSM_InitializationHandlerFunction, (void *)(&HDSMInitializationInfo),  HDSM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HFRM
   { HFRM_InitializationHandlerFunction, (void *)(&HFRMInitializationInfo),  HFRM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HIDM
   { HIDM_InitializationHandlerFunction, NULL,                               HIDM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HOGM
   { HOGM_InitializationHandlerFunction, (void *)(&HOGMInitializationInfo),  HOGM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HRPM
   { HRPM_InitializationHandlerFunction, NULL,                               HRPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HTPM
   { HTPM_InitializationHandlerFunction, NULL,                               HTPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_MAPM
   { MAPM_InitializationHandlerFunction, (void *)(&MAPMInitializationData),  MAPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_OPPM
   { OPPM_InitializationHandlerFunction, NULL,                               OPPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PANM
   { PANM_InitializationHandlerFunction, (void *)(&PANMInitializationInfo),  PANM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PASM
   { PASM_InitializationHandlerFunction, NULL,                               PASM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PBAM
   { PBAM_InitializationHandlerFunction, (void *)(&PBAMInitializationInfo),  PBAM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PXPM
   { PXPM_InitializationHandlerFunction, (void *)(&PXPMInitializationInfo),  PXPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_RSCM
   { RSCM_InitializationHandlerFunction, NULL,                               RSCM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_TDSM
   { TDSM_InitializationHandlerFunction, NULL,                               TDSM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_TIPM
   { TIPM_InitializationHandlerFunction, (void *)(&TIPMInitializationInfo),  TIPM_DeviceManagerHandlerFunction },
#endif
   { NULL                              , NULL,                               NULL                              }
} ;

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Module Handler Service.  This function */
   /* returns a pointer to a Module Handler Entry List (or NULL if there*/
   /* were no Modules installed).  The returned list will simply be an  */
   /* array of Module Handler Entries, with the last entry in the list  */
   /* signified by an entry that has NULL present for all Module Handler*/
   /* Functions.                                                        */
MOD_ModuleHandlerEntry_t *BTPSAPI MOD_GetModuleList(void)
{
   MOD_ModuleHandlerEntry_t *ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MODULE_MANAGER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = ModuleHandlerList;

   DebugPrint((BTPM_DEBUG_ZONE_MODULE_MANAGER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   /* Simply return the Module Handler List.                            */
   return(ret_val);
}

