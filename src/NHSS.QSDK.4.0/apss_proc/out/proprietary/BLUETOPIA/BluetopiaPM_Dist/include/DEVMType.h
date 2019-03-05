/*****< devmtype.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DEVMTYPE - Local Device Manager API Type Definitions and Constants for    */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/14/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __DEVMTYPEH__
#define __DEVMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */
#include "TMRAPI.h"              /* BTPM Timer Prototypes/Constants.          */

#if BTPM_CONFIGURATION_DEVICE_MANAGER_SUPPORT_LOW_ENERGY
   #include "SS1BTGAT.h"         /* Low Energy GATT Prototypes/Constants.     */
#else
   #define GATT_Service_Discovery_Indication_Data_t            void
#endif

   /* The following MACRO is used to determine whether DEVM should      */
   /* 4.2 Feature support.                                              */
#define DEVM_SUPPORT_4_2                        ((BTPS_VERSION_MAJOR_VERSION_NUMBER > 4) || (BTPS_VERSION_MAJOR_VERSION_NUMBER == 4 && BTPS_VERSION_MINOR_VERSION_NUMBER >= 2))

#include "DEVMMSG.h"             /* BTPM Device Manager Message Formats.      */

   /* The following enumerated type respresents the available types of  */
   /* Bluetooth Low Energy (BLE) Address types.                         */
typedef enum
{
   atPublic,
   atStatic,
   atPrivate_Resolvable,
   atPrivate_NonResolvable
} AddressType_t;

   /* The following structure is used to hold information pertaining to */
   /* the current State of the Local Device.                            */
typedef struct _tagDEVM_Local_Device_Properties_t
{
   BD_ADDR_t         BD_ADDR;
   Class_of_Device_t ClassOfDevice;
   unsigned int      DeviceNameLength;
   char              DeviceName[MAX_NAME_LENGTH + 1];
   unsigned int      HCIVersion;
   unsigned int      HCIRevision;
   unsigned int      LMPVersion;
   unsigned int      LMPSubVersion;
   unsigned int      DeviceManufacturer;
   unsigned long     LocalDeviceFlags;
   Boolean_t         DiscoverableMode;
   unsigned int      DiscoverableModeTimeout;
   Boolean_t         ConnectableMode;
   unsigned int      ConnectableModeTimeout;
   Boolean_t         PairableMode;
   unsigned int      PairableModeTimeout;
   Word_t            DeviceAppearance;
   BD_ADDR_t         BLEBD_ADDR;
   AddressType_t     BLEAddressType;
   unsigned int      ScanTimeout;
   unsigned int      AdvertisingTimeout;
} DEVM_Local_Device_Properties_t;

#define DEVM_LOCAL_DEVICE_PROPERTIES_SIZE                         (sizeof(DEVM_Local_Device_Properties_t))

   /* The following bit definitions are used with the LocalDeviceFlags  */
   /* member of the DEVM_Local_Device_Properties_t structure to denote  */
   /* various State information.                                        */
#define DEVM_LOCAL_DEVICE_FLAGS_DEVICE_DISCOVERY_IN_PROGRESS            0x00000001
#define DEVM_LOCAL_DEVICE_FLAGS_LE_SCANNING_IN_PROGRESS                 0x00010000
#define DEVM_LOCAL_DEVICE_FLAGS_LE_ADVERTISING_IN_PROGRESS              0x00020000
#define DEVM_LOCAL_DEVICE_FLAGS_LE_ROLE_IS_CURRENTLY_SLAVE              0x40000000
#define DEVM_LOCAL_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY              0x80000000
#define DEVM_LOCAL_DEVICE_FLAGS_DEVICE_SUPPORTS_ANT_PLUS                0x01000000
#define DEVM_LOCAL_DEVICE_FLAGS_LE_OBSERVATION_IN_PROGRESS              0x02000000
#define DEVM_LOCAL_DEVICE_FLAGS_LE_INTERLEAVED_ADV_SCHEDULING_ENABLED   0x04000000

   /* The following structure holds all pertinent information about the */
   /* Device ID Information of the Local Device.                        */
typedef struct _tagDEVM_Device_ID_Information_t
{
   Word_t    VendorID;
   Word_t    ProductID;
   Word_t    DeviceVersion;
   Boolean_t USBVendorID;
} DEVM_Device_ID_Information_t;

#define DEVM_DEVICE_ID_INFORMATION_SIZE                           (sizeof(DEVM_Device_ID_Information_t))

   /* The following structure is used to hold information needed to     */
   /* start a advertising process.                                      */
   /* * NOTE * The AdvertisingData must be a formatted array of Length  */
   /*          (1 byte) - Tag (1 byte) - Value (arbitrary) structures.  */
typedef struct _tagDEVM_Advertising_Information_t
{
   unsigned long  AdvertisingFlags;
   unsigned long  AdvertisingDuration;
   unsigned int   AdvertisingDataLength;
   Byte_t        *AdvertisingData;
} DEVM_Advertising_Information_t;

   /* The following constants are used with the AdvertisingDuration     */
   /* member of the DEVM_Advertising_Information_t message to specify   */
   /* the Advertising Duration (in seconds).                            */
#define DEVM_ADVERTISING_DURATION_MINIMUM_SECONDS                  0x00000001
#define DEVM_ADVERTISING_DURATION_MAXIMUM_SECONDS                  0xFFFFFFFF

   /* The following bit definitions are used with the AdvertisingFlags  */
   /* member of the DEVM_Advertising_Information_t structure to denote  */
   /* various advertising parameters.                                   */
#define DEVM_ADVERTISING_INFORMATION_FLAGS_USE_PUBLIC_ADDRESS      0x00000001
#define DEVM_ADVERTISING_INFORMATION_FLAGS_DISCOVERABLE            0x00000002
#define DEVM_ADVERTISING_INFORMATION_FLAGS_CONNECTABLE             0x00000004
#define DEVM_ADVERTISING_INFORMATION_FLAGS_ADVERTISE_DEVICE_NAME   0x00000010
#define DEVM_ADVERTISING_INFORMATION_FLAGS_ADVERTISE_TX_POWER      0x00000020
#define DEVM_ADVERTISING_INFORMATION_FLAGS_ADVERTISE_APPEARANCE    0x00000040
#define DEVM_ADVERTISING_INFORMATION_FLAGS_EXTENDED_PACKETS        0x00000080

   /* The following structure is used with the                          */
   /* DEVM_Remote_Device_Properties_t structure to hold application     */
   /* specific information.  This information is set by the manipulated */
   /* by the client application framework and is simply stored in the   */
   /* server framework (including being stored with the other Remote    */
   /* Device information).                                              */
typedef struct _tagDEVM_Remote_Device_Application_Data_t
{
   unsigned int  FriendlyNameLength;
   char          FriendlyName[MAX_NAME_LENGTH + 1];
   unsigned long ApplicationInfo;
} DEVM_Remote_Device_Application_Data_t;

#define DEVM_REMOTE_DEVICE_APPLICATION_DATA_SIZE                  (sizeof(DEVM_Remote_Device_Application_Data_t))

   /* The following structure is used to hold information pertaining to */
   /* the current State of a specific Remote Device.                    */
typedef struct _tagDEVM_Remote_Device_Properties_t
{
   BD_ADDR_t                             BD_ADDR;
   Class_of_Device_t                     ClassOfDevice;
   unsigned int                          DeviceNameLength;
   char                                  DeviceName[MAX_NAME_LENGTH + 1];
   unsigned long                         RemoteDeviceFlags;
   int                                   RSSI;
   int                                   TransmitPower;
   unsigned int                          SniffInterval;
   DEVM_Remote_Device_Application_Data_t ApplicationData;
   AddressType_t                         BLEAddressType;
   int                                   LE_RSSI;
   int                                   LETransmitPower;
   Word_t                                DeviceAppearance;
   BD_ADDR_t                             PriorResolvableBD_ADDR;
   TMR_TimeStamp_t                       BLELastObservedTime;
   GAP_LE_Advertising_Report_Type_t      BLELastAdvertisingPacket;
} DEVM_Remote_Device_Properties_t;

#define DEVM_REMOTE_DEVICE_PROPERTIES_SIZE                        (sizeof(DEVM_Remote_Device_Properties_t))

   /* The following bit definitions are used with the RemoteDeviceFlags */
   /* member of the DEVM_Remote_Device_Properties_t structure to denote */
   /* various State information about the Remote Device.                */
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_NAME_KNOWN                  0x00000001
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_APPLICATION_DATA_VALID      0x00000002
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED            0x00000004
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED         0x00000008
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_ENCRYPTED    0x00000010
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_SNIFF_MODE   0x00000020
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_INITIATED_LOCALLY      0x00000040
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_AUTHENTICATED_KEY           0x00000080
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SERVICES_KNOWN              0x00000100
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_TX_POWER_KNOWN              0x00000200
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_EIR_DATA_KNOWN              0x00000400
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN           0x00001000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_APPEARANCE_KNOWN         0x00002000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE    0x00004000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE 0x00008000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED 0x00010000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_AUTHENTICATED_KEY        0x00020000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_INITIATED_LOCALLY   0x00040000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_TX_POWER_KNOWN           0x00080000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LAST_OBSERVED_KNOWN      0x00100000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LAST_ADV_PACKET_KNOWN    0x00200000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_IS_BR_EDR_MASTER            0x00400000

#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY         0x40000000
#define DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_BR_EDR             0x80000000

   /* The following structure is used to hold extended information for  */
   /* the User Confirmation Request Authentication Action               */
   /* (DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST).           */
   /* * NOTE * The local device manager has been updated to allow the   */
   /*          ability for the authentication handler to alter it's     */
   /*          handling of the the User Confirmation Request action by  */
   /*          being able to determine the remote I/O Capabilities of   */
   /*          the remote device.  Prior to this, it was the            */
   /*          authentication handler's responsibility to note the      */
   /*          Remote I/O Capabilities for use later.                   */
typedef struct _tagDEVM_User_Confirmation_Request_Data_t
{
   DWord_t               Passkey;
   GAP_IO_Capabilities_t IOCapabilities;
} DEVM_User_Confirmation_Request_Data_t;

#define DEVM_USER_CONFIRMATION_REQUEST_DATA_SIZE                  (sizeof(DEVM_User_Confirmation_Request_Data_t))

   /* The following structure is used to hold extended information for  */
   /* the LE IO Capability Response Authentication Action               */
   /* (DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK |           */
   /* DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE).             */
typedef struct _tagDEVM_LE_IO_Capability_Response_Data_t
{
   GAP_LE_IO_Capability_t IO_Capability;
   Boolean_t              OOB_Present;
   GAP_LE_Bonding_Type_t  Bonding_Type;
   Boolean_t              MITM;
} DEVM_LE_IO_Capability_Response_Data_t;

#define DEVM_LE_IO_CAPABILITY_RESPONSE_DATA_SIZE                  (sizeof(DEVM_LE_IO_Capability_Response_Data_t))

   /* The following structure is used as a container structure to hold  */
   /* Authentication Information that is required when authentication a */
   /* remote device (either initiated locally or remotely).             */
   /* The Authentication Action member defines which member of the      */
   /* AuthenticationData union is valid:                                */
   /*  if DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK is NOT   */
   /*  set:                                                             */
   /*    - DEVM_AUTHENTICATION_ACTION_PIN_CODE_REQUEST                  */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_PIN_CODE_RESPONSE                 */
   /*       - PINCode member is valid/used.                             */
   /*    - DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST         */
   /*       - Passkey member is valid/used.                             */
   /*            or                                                     */
   /*       - UserConfirmationRequestData member is valid/used.         */
   /*    - DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE        */
   /*       - Confirmation member is valid/used.                        */
   /*    - DEVM_AUTHENTICATION_ACTION_PASSKEY_REQUEST                   */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_PASSKEY_RESPONSE                  */
   /*       - PassKey member is valid/used.                             */
   /*    - DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION                */
   /*       - PassKey member is valid/used.                             */
   /*    - DEVM_AUTHENTICATION_ACTION_KEYPRESS_INDICATION               */
   /*       - Kepress member is valid/used.                             */
   /*    - DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST          */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE         */
   /*       - OutOfBandData member is valid/used.                       */
   /*    - DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST           */
   /*       - IOCapabilities member is valid/used.                      */
   /*    - DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE          */
   /*       - IOCapabilities member is valid/used.                      */
   /*    - DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_STATUS_RESULT      */
   /*       - AuthenticationStatus member is valid/used.                */
   /*    - DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_START              */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_END                */
   /*       - AuthenticationStatus member is valid/used.                */
   /*                                                                   */
   /*  if DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK IS       */
   /*  set:                                                             */
   /*    - DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST           */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE          */
   /*       - LEIOCapabilities is used.                                 */
   /*    - DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST         */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE        */
   /*       - Confirmation is used.                                     */
   /*    - DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION                */
   /*       - Passkey is valid.                                         */
   /*    - DEVM_AUTHENTICATION_ACTION_PASSKEY_REQUEST                   */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_PASSKEY_RESPONSE                  */
   /*       - Passkey is used.                                          */
   /*    - DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST          */
   /*       - No member is valid/used.                                  */
   /*    - DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE         */
   /*       - LEOutOfBandData is used.                                  */
   /* * NOTE * If a rejection is to be specified (for a response) then  */
   /*          the AuthenticationDataLength member should be set to     */
   /*          zero.  Otherwise, for a non-rejection, and valid data,   */
   /*          the AuthenticationDataLength member should be set to     */
   /*          size of the element in the Authentication_Data union     */
   /*          that is being used.                                      */
   /* * NOTE * For actions that are specified to have more than a single*/
   /*          type of data member (for instance the                    */
   /*          DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST     */
   /*          action), the size can be used to determine the data type */
   /*          that should be used.                                     */
   /* * NOTE * The Flags member is only valid when using a 4.2          */
   /*          controller.                                              */
typedef struct _tagDEVM_Authentication_Information_t
{
   BD_ADDR_t                                BD_ADDR;
   unsigned int                             AuthenticationAction;
   unsigned int                             AuthenticationDataLength;
   union
   {
      Byte_t                                AuthenticationStatus;
      PIN_Code_t                            PINCode;
      Boolean_t                             Confirmation;
      DWord_t                               Passkey;
      GAP_Keypress_t                        Keypress;
      GAP_Out_Of_Band_Data_t                OutOfBandData;
      GAP_IO_Capabilities_t                 IOCapabilities;
      DEVM_LE_IO_Capability_Response_Data_t LEIOCapabilities;
      GAP_LE_OOB_Data_t                     LEOutOfBandData;
      DEVM_User_Confirmation_Request_Data_t UserConfirmationRequestData;
   } AuthenticationData;
   unsigned long                            Flags;
} DEVM_Authentication_Information_t;

#define DEVM_AUTHENTICATION_INFORMATION_SIZE                      (sizeof(DEVM_Authentication_Information_t))

#define DEVM_AUTHENTICATION_INFORMATION_FLAGS_SECURE_CONNECTIONS  0x00000001
#define DEVM_AUTHENTICATION_INFORMATION_FLAGS_JUST_WORKS_PAIRING  0x00000002

   /* This structure is a subset of the above structure.  This is the   */
   /* original Authentication Information structure that was supported  */
   /* by the Device Manager.  See comments above for a description of   */
   /* the individual fields in this message.                            */
   /* * NOTE * Clients can look at the Message size to determine what   */
   /*          structure that has been returned from the server.        */
typedef struct _tagDEVM_Authentication_Information_Legacy_1_t
{
   BD_ADDR_t                 BD_ADDR;
   unsigned int              AuthenticationAction;
   unsigned int              AuthenticationDataLength;
   union
   {
      Byte_t                 AuthenticationStatus;
      PIN_Code_t             PINCode;
      Boolean_t              Confirmation;
      DWord_t                Passkey;
      GAP_Keypress_t         Keypress;
      GAP_Out_Of_Band_Data_t OutOfBandData;
      GAP_IO_Capabilities_t  IOCapabilities;
   } AuthenticationData;
} DEVM_Authentication_Information_Legacy_1_t;

#define DEVM_AUTHENTICATION_INFORMATION_LEGACY_1_SIZE             (sizeof(DEVM_Authentication_Information_Legacy_1_t))

   /* The following constants represent the defined values that are     */
   /* allowable for the AuthenticationAction member of the              */
   /* DEVM_Authentication_Information_t structure.                      */
   /* * NOTE * The AuthenticationAction member contains two parts:      */
   /*             - The actual action (Lowest order bits)               */
   /*             - The operation type (Low Energy or BR/EDR)           */
#define DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_ACTION_MASK     0x7FFFFFFF
#define DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK      0x80000000

#define DEVM_AUTHENTICATION_ACTION_PIN_CODE_REQUEST               0x00000001
#define DEVM_AUTHENTICATION_ACTION_PIN_CODE_RESPONSE              0x00000002
#define DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST      0x00000003
#define DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE     0x00000004
#define DEVM_AUTHENTICATION_ACTION_PASSKEY_REQUEST                0x00000005
#define DEVM_AUTHENTICATION_ACTION_PASSKEY_RESPONSE               0x00000006
#define DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION             0x00000007
#define DEVM_AUTHENTICATION_ACTION_KEYPRESS_INDICATION            0x00000008
#define DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST       0x00000009
#define DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE      0x0000000A
#define DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST        0x0000000B
#define DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE       0x0000000C
#define DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_STATUS_RESULT   0x0000000D

#define DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_START           0x00000010
#define DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_END             0x00000011

   /* The following structure is used to hold information pertaining to */
   /* an observation process.  The ObservationParameterFlags specifies  */
   /* which optional information in the structure is valid.             */
   /* * NOTE * The ReportingFrequency member (NOT optional, minimum     */
   /*          value is                                                 */
   /*                                                                   */
   /*BTPM_CONFIGURATION_DEVICE_MANAGER_MIN_OBSERVATION_REPORTING_TIME_MS*/
   /*                                                                   */
   /*          specifies the minimum amount of time (in milliseconds)   */
   /*          between reporting an observation of a particular device. */
   /*          For example, if ReportingFrequency = 100ms then a device */
   /*          will only be reported to the application as observed at  */
   /*          most once every 100ms even if the device is observed 5   */
   /*          times in a 100 ms period. This should be set to the      */
   /*          granularity needed by the application.                   */
   /* * NOTE * If the                                                   */
   /*                                                                   */
   /*       DEVM_OBSERVATION_PARAMETER_FLAGS_SCAN_WINDOW_INTERVAL_VALID */
   /*                                                                   */
   /*          flag is set in the ObservationParameterFlags member then */
   /*          both the ScanWindow and ScanInterval members must contain*/
   /*          valid values within the range 3-10240 ms (inclusive).    */
   /*          The ScanWindow member must be less than or equal to      */
   /*          ScanInterval.                                            */
typedef struct _tagDEVM_Observation_Parameters_t
{
   unsigned long ObservationParameterFlags;
   unsigned int  ReportingFrequency;
   unsigned int  ScanWindow;
   unsigned int  ScanInterval;
} DEVM_Observation_Parameters_t;

#define DEVM_OBSERVATION_PARAMETERS_SIZE                         (sizeof(DEVM_Observation_Parameters_t))

   /* The following bit definitions are used with the                   */
   /* ObservationInfoFlags member of the DEVM_Observation_Information_t */
   /* structure to which members of the structure are valid.            */
#define DEVM_OBSERVATION_PARAMETER_FLAGS_SCAN_WINDOW_INTERVAL_VALID 0x00000001

   /* The following enumeration is used to specify roles when performing*/
   /* a Role Switch.                                                    */
typedef enum
{
   drMaster,
   drSlave
} DEVM_Role_t;

#endif
