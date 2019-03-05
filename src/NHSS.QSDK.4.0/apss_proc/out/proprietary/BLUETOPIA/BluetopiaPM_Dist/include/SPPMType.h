/*****< sppmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPMTYPE - Serial Port Profile Manager API Type Definitions and Constants */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/20/12  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SPPMTYPEH__
#define __SPPMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SPPMMSG.h"             /* BTPM Serial Port Manager Message Formats. */

   /* The following enumerated type represents the connection types of  */
   /* Serial Port Profile Manager (SPPM) connections that are supported */
   /* by the Serial Port Profile Manager.                               */
typedef enum
{
   sctSPP,
   sctMFi
} SPPM_Connection_Type_t;

   /* The following structure represents an individual MFi Full ID      */
   /* String (FID) Token.  A Token consists of the FID Type/Subtype     */
   /* followed by the FID Data.  This structure is used when specifying */
   /* the MFi configuration settings (via the                           */
   /* SPPM_ConfigureMFiSettings() function).                            */
typedef struct _tagSPPM_MFi_FID_Token_Value_t
{
   Byte_t  FIDType;
   Byte_t  FIDSubType;
   Byte_t  FIDDataLength;
   Byte_t *FIDData;
} SPPM_MFi_FID_Token_Value_t;

#define SPPM_MFI_FID_TOKEN_VALUE_SIZE                          (sizeof(SPPM_MFi_FID_Token_Value_t))

   /* The following enumerated type represents the defined Identify     */
   /* Device Preferences and Settings (IDPS) states (for reporting the  */
   /* status).                                                          */
typedef enum
{
   isStartIdentificationRequest,
   isStartIdentificationProcess,
   isIdentificationProcess,
   isIdentificationProcessComplete,
   isStartAuthenticationProcess,
   isAuthenticationProcess,
   isAuthenticationProcessComplete
} SPPM_IDPS_State_t;

   /* The following constants represent various maximum supported       */
   /* features of this implementation.                                  */
#define SPPM_MFI_MAXIMUM_SUPPORTED_LINGOS                      255
#define SPPM_MFI_MAXIMUM_SUPPORTED_PROTOCOLS                    32
#define SPPM_MFI_MAXIMUM_SUPPORTED_LANGUAGES                   255

#define SPPM_MFI_MAXIMUM_SUPPORTED_MANUFACTURER_NAME_LENGTH     64
#define SPPM_MFI_MAXIMUM_SUPPORTED_MODEL_NUMBER_LENGTH          64
#define SPPM_MFI_MAXIMUM_SUPPORTED_SERIAL_NUMBER_LENGTH         64
#define SPPM_MFI_MAXIMUM_SUPPORTED_BUNDLE_SEED_ID_LENGTH        11
#define SPPM_MFI_MAXIMUM_SUPPORTED_PROTOCOL_STRING_LENGTH       92
#define SPPM_MFI_MAXIMUM_SUPPORTED_LANGUAGE_ID_STRING_LENGTH     4

#define SPPM_MFI_RECEIVE_PACKET_SIZE_MINIMUM                   256
#define SPPM_MFI_RECEIVE_PACKET_SIZE_MAXIMUM                 65000

#define SPPM_MFI_PACKET_TIMEOUT_MINIMUM_MS                     500
#define SPPM_MFI_PACKET_TIMEOUT_MAXIMUM_MS                   15000

   /* The following structure is a container structure which is used    */
   /* with the SPPM_MFi_Configuration_Settings_t structure to denote the*/
   /* various Accessory Inforamation (required fields for MFi           */
   /* configuration).                                                   */
   /* * NOTE * The AccessoryInformationBitMask member is a bit mask     */
   /*          that specifies which of the following optional fields    */
   /*          are valid.                                               */
   /* * NOTE * The Manufacturer, Model Number, and Serial Number members*/
   /*          are NULL terminated UTF-8 character strings that can be  */
   /*          up to the length specified (including NULL terminator).  */
   /*          The length specified specifies the largest length,       */
   /*          however the strings can be shorter.                      */
typedef struct _tagSPPM_MFi_Accessory_Info_t
{
   QWord_t      AccessoryCapabilitiesBitmask;
   Byte_t       AccessoryName[64];
   unsigned int AccessoryInformationBitMask;
   Byte_t       AccessoryFirmwareVersion[3];
   Byte_t       AccessoryHardwareVersion[3];
   Byte_t       AccessoryManufacturer[SPPM_MFI_MAXIMUM_SUPPORTED_MANUFACTURER_NAME_LENGTH];
   Byte_t       AccessoryModelNumber[SPPM_MFI_MAXIMUM_SUPPORTED_MODEL_NUMBER_LENGTH];
   Byte_t       AccessorySerialNumber[SPPM_MFI_MAXIMUM_SUPPORTED_SERIAL_NUMBER_LENGTH];
   DWord_t      AccessoryRFCertification;
} SPPM_MFi_Accessory_Info_t;

   /* The following constants are used with the                         */
   /* AccessoryInformationBitMask member of the                         */
   /* SPPM_MFi_Accessory_Info_t structure to denote which fields are    */
   /* valid.                                                            */
#define SPPM_MFI_ACCESSORY_INFORMATION_BITMASK_FIRMWARE_VERSION      0x01
#define SPPM_MFI_ACCESSORY_INFORMATION_BITMASK_HARDWARE_VERSION      0x02
#define SPPM_MFI_ACCESSORY_INFORMATION_BITMASK_MANUFACTURER_NAME     0x04
#define SPPM_MFI_ACCESSORY_INFORMATION_BITMASK_MODEL_NUMBER          0x08
#define SPPM_MFI_ACCESSORY_INFORMATION_BITMASK_SERIAL_NUMBER         0x10

   /* The following enumerated type represents the defined options for  */
   /* locating an app which handles a particular protocol.              */
typedef enum
{
   maNone,
   maAutomaticSearch,
   maSearchButtonOnly
} SPPM_MFi_Match_Action_t;

   /* The following structure is the container structure for an MFi     */
   /* Protocol string.  This string is an NULL terminated UTF-8         */
   /* character string.                                                 */
   /* * NOTE * The ProtocolString member is a NULL terminated UTF-8     */
   /*          character string that can be up to the length specified  */
   /*          (including NULL terminator).  The length specified       */
   /*          specifies the largest length, however the string can be  */
   /*          shorter.                                                 */
typedef struct _tagSPPM_MFi_Protocol_String_t
{
   Byte_t                  ProtocolString[SPPM_MFI_MAXIMUM_SUPPORTED_PROTOCOL_STRING_LENGTH];
   SPPM_MFi_Match_Action_t MatchAction;
} SPPM_MFi_Protocol_String_t;

   /* The following structure is the container structure for an MFi     */
   /* Language Identifier string.  This string is a NULL terminated     */
   /* UTF-8 character string consisting of either an alpha-2 or alpha-3 */
   /* language identifier string, as defined by ISO 639-1 and ISO 639-2,*/
   /* respectively.                                                     */
   /* * NOTE * The LanguageID member is a NULL terminated UTF-8         */
   /*          character string that can be up to the length specified  */
   /*          (including NULL terminator).  The length specified       */
   /*          specifies the largest length, however the string can be  */
   /*          shorter.                                                 */
typedef struct _tagSPPM_MFi_Language_ID_String_t
{
   Byte_t LanguageID[SPPM_MFI_MAXIMUM_SUPPORTED_LANGUAGE_ID_STRING_LENGTH];
} SPPM_MFi_Language_ID_String_t;

   /* The following structure is used with the                          */
   /* SPPM_ConfigureMFiSettings() function to inform the Serial Port    */
   /* Profile Manager (SPPM) of the configured MFi capabilities.        */
   /* * NOTE * The General Lingo is always supported and does not have  */
   /*          to be specified in the Supported Lingo list.             */
   /* * NOTE * All fields are mandatory to specify except:              */
   /*             - SupportedLingoList (only required if other Lingos   */
   /*               besides General Lingo is supported).                */
   /*             - SupportedProtocols (only required if device supports*/
   /*               specific protocols (used with the BundleSeed).      */
   /*             - FIDTokenList (only required if other FID tokens are */
   /*               required).                                          */
   /*             - ControlMessagesSent (only required if device        */
   /*               supports more than the default messages)            */
   /*             - ControlMessagesReceived (only required if device    */
   /*               supports more than the default messages)            */
   /* * NOTE * If ControlMessagesSent is *NOT* specified, then the      */
   /*          device will automatically report support for sending the */
   /*          following messages:                                      */
   /*             - RequestAppLaunch                                    */
   /* * NOTE * If ControlMessagesReceived is *NOT* specified, then the  */
   /*          device will automatically report support for sending the */
   /*          following messages:                                      */
   /*             - StartExternalAccessoryProtocolSession               */
   /*             - StopExternalAccessoryProtocolSession                */
   /* * NOTE * If either ControlMessagesSent or ControlMessagesReceived */
   /*          *IS* specified, then the specified list *MUST* include   */
   /*          the default control messages (listed above) if support   */
   /*          for them is required by the device.                      */
typedef struct _tagSPPM_MFi_Configuration_Settings_t
{
   unsigned int                        MaximumReceivePacketSize;
   unsigned int                        DataPacketTimeout;
   Byte_t                              NumberSupportedLingos;
   Byte_t                             *SupportedLingoList;
   SPPM_MFi_Accessory_Info_t           AccessoryInfo;
   Byte_t                              NumberSupportedProtocols;
   SPPM_MFi_Protocol_String_t         *SupportedProtocolList;
   Byte_t                              BundleSeedIDString[SPPM_MFI_MAXIMUM_SUPPORTED_BUNDLE_SEED_ID_LENGTH];
   Byte_t                              NumberFIDTokens;
   SPPM_MFi_FID_Token_Value_t         *FIDTokenList;
   SPPM_MFi_Language_ID_String_t       CurrentLanguage;
   Byte_t                              NumberSupportedLanguages;
   SPPM_MFi_Language_ID_String_t      *SupportedLanguagesList;
   Word_t                              NumberControlMessagesSent;
   Word_t                             *ControlMessagesSentIDList;
   Word_t                              NumberControlMessagesReceived;
   Word_t                             *ControlMessagesReceivedIDList;
} SPPM_MFi_Configuration_Settings_t;

#define SPPM_MFI_CONFIGURATION_SETTINGS_SIZE                   (sizeof(SPPM_MFi_Configuration_Settings_t))

#endif
