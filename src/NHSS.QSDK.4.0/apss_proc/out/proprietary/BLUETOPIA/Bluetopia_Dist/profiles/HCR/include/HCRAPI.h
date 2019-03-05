/*****< hcrapi.h >*************************************************************/
/*      Copyright 2002 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HCRAPI - Stonestreet One Bluetooth Stack Hard Copy Cable Replacement      */
/*           (HCRP) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/30/02  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __HCRAPIH__
#define __HCRAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

#include "HCRTypes.h"           /* Bluetooth HCRP Type Definitions/Constants. */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTHCR_ERROR_INVALID_PARAMETER                             (-1000)
#define BTHCR_ERROR_NOT_INITIALIZED                               (-1001)
#define BTHCR_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTHCR_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTHCR_ERROR_OUTSTANDING_TRANSACTION                       (-1005)
#define BTHCR_ERROR_CREDIT_OVERFLOW                               (-1006)
#define BTHCR_ERROR_OUTGOING_MTU_EXCEEDED                         (-1007)
#define BTHCR_ERROR_NOT_ALLOWED_WHILE_CONNECTED                   (-1008)

   /* SDP Profile UUID's for the Hard Copy Cable Replacement Profile.   */

   /* SDP Hard Copy Cable Replacement Profile UUID's.                   */

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Profile Bluetooth Universally Unique Identifier */
   /* (HARD_CABLE_REPLACEMENT_PROFILE_UUID_16) to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is    */
   /* the UUID_16_t variable that is to receive the                     */
   /* HARD_CABLE_REPLACEMENT_PROFILE_UUID_16 Constant value.            */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_PROFILE_UUID_16(_x)        ASSIGN_SDP_UUID_16((_x), 0x11, 0x25)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Profile Bluetooth Universally Unique Identifier */
   /* (HARD_CABLE_REPLACEMENT_PROFILE_UUID_32) to the specified         */
   /* UUID_32_t variable.  This MACRO accepts one parameter which is    */
   /* the UUID_32_t variable that is to receive the                     */
   /* HARD_CABLE_REPLACEMENT_PROFILE_UUID_32 Constant value.            */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_PROFILE_UUID_32(_x)        ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x25)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Profile Bluetooth Universally Unique Identifier */
   /* (HARD_CABLE_REPLACEMENT_PROFILE_UUID_128) to the specified        */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* HARD_CABLE_REPLACEMENT_PROFILE_UUID_128 Constant value.           */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_PROFILE_UUID_128(_x)       ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x25, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Hard Copy Cable Replacement Service Class UUID's.             */

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Print Service Class Bluetooth Universally Unique*/
   /* Identifier (HARD_CABLE_REPLACEMENT_PRINT_UUID_16) to the specified*/
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the                         */
   /* HARD_CABLE_REPLACEMENT_PRINT_UUID_16 Constant value.              */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_PRINT_UUID_16(_x)          ASSIGN_SDP_UUID_16((_x), 0x11, 0x26)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Print Service Class Bluetooth Universally Unique*/
   /* Identifier (HARD_CABLE_REPLACEMENT_PRINT_UUID_32) to the specified*/
   /* UUID_32_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_32_t variable that is to receive the                         */
   /* HARD_CABLE_REPLACEMENT_PRINT_UUID_32 Constant value.              */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_PRINT_UUID_32(_x)          ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x26)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Print Service Class Bluetooth Universally Unique*/
   /* Identifier (HARD_CABLE_REPLACEMENT_PRINT_UUID_128) to the         */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* HARD_CABLE_REPLACEMENT_PRINT_UUID_128 Constant value.             */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_PRINT_UUID_128(_x)         ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x26, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Scan Service Class Bluetooth Universally Unique */
   /* Identifier (HARD_CABLE_REPLACEMENT_SCAN_UUID_16) to the specified */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the                         */
   /* HARD_CABLE_REPLACEMENT_SCAN_UUID_16 Constant value.               */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_SCAN_UUID_16(_x)           ASSIGN_SDP_UUID_16((_x), 0x11, 0x27)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Scan Service Class Bluetooth Universally Unique */
   /* Identifier (HARD_CABLE_REPLACEMENT_SCAN_UUID_32) to the specified */
   /* UUID_32_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_32_t variable that is to receive the                         */
   /* HARD_CABLE_REPLACEMENT_SCAN_UUID_32 Constant value.               */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_SCAN_UUID_32(_x)           ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x27)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement Scan Service Class Bluetooth Universally Unique */
   /* Identifier (HARD_CABLE_REPLACEMENT_SCAN_UUID_128) to the          */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* HARD_CABLE_REPLACEMENT_SCAN_UUID_128 Constant value.              */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_SCAN_UUID_128(_x)          ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x27, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Hard Copy Cable Replacement Protocol UUID's.                  */

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Control Channel Bluetooth Universally     */
   /* Unique Identifier (HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_16)*/
   /* to the specified UUID_16_t variable.  This MACRO accepts one      */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_16 Constant value.    */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_16(_x) ASSIGN_SDP_UUID_16((_x), 0x00, 0x12)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Control Channel Bluetooth Universally     */
   /* Unique Identifier (HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_32)*/
   /* to the specified UUID_32_t variable.  This MACRO accepts one      */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_32 Constant value.    */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_32(_x) ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x12)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Control Channel Bluetooth Universally     */
   /* Unique Identifier                                                 */
   /* (HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_128) to the specified*/
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_128 Constant value.   */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_CONTROL_CHANNEL_UUID_128(_x) ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Data Channel Bluetooth Universally Unique */
   /* Identifier (HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_16) to the   */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_16 Constant value.       */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_16(_x) ASSIGN_SDP_UUID_16((_x), 0x00, 0x14)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Data Channel Bluetooth Universally Unique */
   /* Identifier (HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_32) to the   */
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the            */
   /* HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_32 Constant value.       */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_32(_x) ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x14)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Data Channel Bluetooth Universally Unique */
   /* Identifier (HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_128) to the  */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_128 Constant value.      */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_DATA_CHANNEL_UUID_128(_x) ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Notification Channel Bluetooth Universally*/
   /* Unique Identifier                                                 */
   /* (HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_16) to the      */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_16 Constant      */
   /* value.                                                            */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_16(_x) ASSIGN_SDP_UUID_16((_x), 0x00, 0x16)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Notification Channel Bluetooth Universally*/
   /* Unique Identifier                                                 */
   /* (HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_32) to the      */
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the            */
   /* HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_32 Constant      */
   /* value.                                                            */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_32(_x) ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x16)

   /* The following MACRO is a utility MACRO that assigns the Hard Copy */
   /* Cable Replacement L2CAP Notification Channel Bluetooth Universally*/
   /* Unique Identifier                                                 */
   /* (HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_128) to the     */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_128 Constant     */
   /* value.                                                            */
#define SDP_ASSIGN_HARD_CABLE_REPLACEMENT_NOTIFICATION_CHANNEL_UUID_128(_x) ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* Hard Cable Repleacement Profile Servers.                          */
#define HCR_PROFILE_VERSION                                          (0x0102)

   /* Hard Copy Cable Replacement Constants.                            */
#define HCR_SERVICE_NAME_MAXIMUM_LENGTH                              248

#define HCR_DEVICE_NAME_MAXIMUM_LENGTH                               248

#define HCR_FRIENDLY_NAME_MAXIMUM_LENGTH                             248

#define HCR_DEVICE_LOCATION_MAXIMUM_LENGTH                           248

#define HCR_IEEE_1284_ID_STRING_MAXIMUM_LENGTH                     65535

   /* The following constants represent the IEEE 1284 Job Status Bits   */
   /* that are used with the Retrieving of the HCR Server's LPT Status. */
   /* * NOTE * All Bits that are not explicitly listed are Reserved     */
   /*          and should be set to zero.                               */
#define HCR_LPT_STATUS_PAPER_EMPTY_BIT                               0x20
#define HCR_LPT_STATUS_SELECT_BIT                                    0x10
#define HCR_LPT_STATUS_NOT_ERROR_BIT                                 0x08

   /* The following structure is used with the                          */
   /* HCR_Register_Server_SDP_Record() function to describe the IEEE    */
   /* 1284ID String that should be added to the SDP Record for the HCR  */
   /* Server.                                                           */
typedef struct _tagHCR_SDP_1284_ID_Information_t
{
   Word_t  IEEE_1284IDLength;
   Byte_t *IEEE_1284IDString;
} HCR_SDP_1284_ID_Information_t;

   /* The following enumerated type specifies the different Service     */
   /* Types that are supported by the Hard Copy Cable Replacement       */
   /* Profile.  These types are specified when a HCR Server is created. */
typedef enum
{
   stPrinter,
   stScanner
} HCRServiceType_t;

   /* The following constants represent the Minimum and Maximum PDU ID  */
   /* Values that can be used when sending Vendor Specific Data.        */
#define HCR_PDU_ID_VENDOR_SPECIFIC_MINIMUM                         0x8000
#define HCR_PDU_ID_VENDOR_SPECIFIC_MAXIMUM                         0xFFFF

   /* The following constants represent the HCR PDU Status Reply        */
   /* values that can be sent in a Reply PDU.                           */
#define HCR_PDU_STATUS_FEATURE_UNSUPPORTED                         0x0000
#define HCR_PDU_STATUS_SUCCESS                                     0x0001
#define HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                0x0002
#define HCR_PDU_STATUS_GENERIC_FAILURE                             0xFFFF

   /* The following constants represent the HCR Client Open Status      */
   /* Values that are possible in the HCR Client Connect Confirmation   */
   /* Event.                                                            */
#define HCR_OPEN_STATUS_SUCCESS                                      0x00
#define HCR_OPEN_STATUS_CONNECTION_TIMEOUT                           0x01
#define HCR_OPEN_STATUS_CONNECTION_REFUSED                           0x02
#define HCR_OPEN_STATUS_UNKNOWN_ERROR                                0x03

   /* The following enumerated type represents the supported Server     */
   /* Connection Modes supported by a HCR Server.                       */
typedef enum
{
   csmAutomaticAccept,
   csmAutomaticReject,
   csmManualAccept
} HCR_Server_Connection_Mode_t;

   /* Hard Copy Cable Replacement Event API Types.                      */
typedef enum
{
   etHCR_Control_Connect_Indication,
   etHCR_Data_Connect_Indication,
   etHCR_Notification_Connect_Indication,
   etHCR_Control_Connect_Confirmation,
   etHCR_Data_Connect_Confirmation,
   etHCR_Notification_Connect_Confirmation,
   etHCR_Control_Disconnect_Indication,
   etHCR_Data_Disconnect_Indication,
   etHCR_Notification_Disconnect_Indication,
   etHCR_Credit_Grant_Indication,
   etHCR_Credit_Grant_Confirmation,
   etHCR_Credit_Request_Indication,
   etHCR_Credit_Request_Confirmation,
   etHCR_Credit_Return_Indication,
   etHCR_Credit_Return_Confirmation,
   etHCR_Credit_Query_Indication,
   etHCR_Credit_Query_Confirmation,
   etHCR_LPT_Status_Query_Indication,
   etHCR_LPT_Status_Query_Confirmation,
   etHCR_IEEE_1284_ID_Query_Indication,
   etHCR_IEEE_1284_ID_Query_Confirmation,
   etHCR_Soft_Reset_Indication,
   etHCR_Soft_Reset_Confirmation,
   etHCR_Hard_Reset_Indication,
   etHCR_Hard_Reset_Confirmation,
   etHCR_Register_Notification_Indication,
   etHCR_Register_Notification_Confirmation,
   etHCR_Notification_Connection_Alive_Indication,
   etHCR_Notification_Connection_Alive_Confirmation,
   etHCR_Vendor_Specific_Data_Indication,
   etHCR_Vendor_Specific_Data_Confirmation,
   etHCR_Data_Indication,
   etHCR_Notification_Indication,
   etHCR_Connect_Request_Indication
} HCR_Event_Type_t;

   /* The following event is dispatched when a remote device is         */
   /* requesting a connection to a local HCR service.  The BD_ADDR      */
   /* specifies the Bluetooth Address of the Remote Device that is      */
   /* connecting.                                                       */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            HCR_Connect_Request_Response() function in order to    */
   /*            accept or reject the outstanding connect Request.      */
typedef struct _tagHCR_Connect_Request_Indication_Data_t
{
   unsigned int HCRID;
   BD_ADDR_t    BD_ADDR;
} HCR_Connect_Request_Indication_Data_t;

#define HCR_CONNECT_REQUEST_INDICATION_DATA_SIZE         (sizeof(HCR_Connect_Request_Indication_Data_t))

   /* The following HCR Event is dispatched when a HCR Client Connects  */
   /* to a registered HCR Server (Control Channel Only).  The HCRID     */
   /* member specifies the Local Server that has been connected to and  */
   /* the BD_ADDR member specifies the Client Bluetooth Device that has */
   /* connected to the specified Server.                                */
typedef struct _tagHCR_Control_Connect_Indication_Data_t
{
   unsigned int HCRID;
   BD_ADDR_t    BD_ADDR;
} HCR_Control_Connect_Indication_Data_t;

#define HCR_CONTROL_CONNECT_INDICATION_DATA_SIZE        (sizeof(HCR_Control_Connect_Indication_Data_t))

   /* The following HCR Event is dispatched when a HCR Client Connects  */
   /* to a registered HCR Server (Data Channel Only).  The HCRID        */
   /* member specifies the Local Server that has been connected to and  */
   /* the BD_ADDR member specifies the Client Bluetooth Device that has */
   /* connected to the specified Server.                                */
typedef struct _tagHCR_Data_Connect_Indication_Data_t
{
   unsigned int HCRID;
   BD_ADDR_t    BD_ADDR;
} HCR_Data_Connect_Indication_Data_t;

#define HCR_DATA_CONNECT_INDICATION_DATA_SIZE           (sizeof(HCR_Data_Connect_Indication_Data_t))

   /* The following HCR Event is dispatched when a HCR Client Connects  */
   /* to a registered HCR Notification Server (Notification Channel     */
   /* Only).  The HCRID member specifies the Local Server that has been */
   /* connected to and the BD_ADDR member specifies the Client Bluetooth*/
   /* Device that has connected to the specified Server.                */
typedef struct _tagHCR_Notification_Connect_Indication_Data_t
{
   unsigned int HCRID;
   BD_ADDR_t    BD_ADDR;
} HCR_Notification_Connect_Indication_Data_t;

#define HCR_NOTIFICATION_CONNECT_INDICATION_DATA_SIZE   (sizeof(HCR_Notification_Connect_Indication_Data_t))

   /* The following HCR Event is dispatched when a remote HCR Unit      */
   /* (Client or Server) Disconnects from the Local HCR Control         */
   /* Connection.  The HCRID member specifies the Local HCR Connection  */
   /* that the Remote HCR Unit has disconnected the Control Channel     */
   /* from.                                                             */
   /* * NOTE * Once a Control Channel is disconnected, the Data Channel */
   /*          is Disconnected as well (and vice-versa).                */
typedef struct _tagHCR_Control_Disconnect_Indication_Data_t
{
   unsigned int HCRID;
} HCR_Control_Disconnect_Indication_Data_t;

#define HCR_CONTROL_DISCONNECT_INDICATION_DATA_SIZE      (sizeof(HCR_Control_Disconnect_Indication_Data_t))

   /* The following HCR Event is dispatched when a remote HCR Unit      */
   /* (Client or Server) Disconnects from the Local HCR Data Connection.*/
   /* The HCRID member specifies the Local HCR Connection that the      */
   /* Remote HCR Unit has disconnected the Data Channel from.           */
typedef struct _tagHCR_Data_Disconnect_Indication_Data_t
{
   unsigned int HCRID;
} HCR_Data_Disconnect_Indication_Data_t;

#define HCR_DATA_DISCONNECT_INDICATION_DATA_SIZE         (sizeof(HCR_Data_Disconnect_Indication_Data_t))

   /* The following HCR Event is dispatched when a remote HCR Unit      */
   /* (Client or Server) Disconnects from the Local HCR Notification    */
   /* Connection.  The HCRID member specifies the Local HCR Connection  */
   /* that the Remote HCR Unit has disconnected the Notification Channel*/
   /* from.                                                             */
typedef struct _tagHCR_Notification_Disconnect_Indication_Data_t
{
   unsigned int HCRID;
} HCR_Notification_Disconnect_Indication_Data_t;

#define HCR_NOTIFICATION_DISCONNECT_INDICATION_DATA_SIZE (sizeof(HCR_Notification_Disconnect_Indication_Data_t))

   /* The following HCR Event is dispatched in response to attempting to*/
   /* connect to a Remote HCR Profile (Control Channel Only).  The HCRID*/
   /* member specifies the local HCR Client Connection to the Remote    */
   /* Profile and the ConnectionStatus member specifies the Connection  */
   /* Status.  If the Connection Status is successful then the Control  */
   /* Channel has been successfully established.                        */
typedef struct _tagHCR_Control_Connect_Confirmation_Data_t
{
   unsigned int HCRID;
   unsigned int ConnectionStatus;
} HCR_Control_Connect_Confirmation_Data_t;

#define HCR_CONTROL_CONNECT_CONFIRMATION_DATA_SIZE      (sizeof(HCR_Control_Connect_Confirmation_Data_t))

   /* The following HCR Event is dispatched in response to attempting to*/
   /* connect to a Remote HCR Profile (Data Channel Only).  The HCRID   */
   /* member specifies the local HCR Client Connection to the Remote    */
   /* Profile and the ConnectionStatus member specifies the Connection  */
   /* Status.  If the Connection Status is successful then the Data     */
   /* Channel has been successfully established.                        */
typedef struct _tagHCR_Data_Connect_Confirmation_Data_t
{
   unsigned int HCRID;
   unsigned int ConnectionStatus;
} HCR_Data_Connect_Confirmation_Data_t;

#define HCR_DATA_CONNECT_CONFIRMATION_DATA_SIZE         (sizeof(HCR_Data_Connect_Confirmation_Data_t))

   /* The following HCR Event is dispatched in response to attempting to*/
   /* connect to a Remote HCR Profile (Notification Channel Only).  The */
   /* HCRID member specifies the local HCR Client Connection to the     */
   /* Remote Profile and the ConnectionStatus member specifies the      */
   /* Connection Status.  If the Connection Status is successful then   */
   /* the Notification Channel has been successfully established.       */
typedef struct _tagHCR_Notification_Connect_Confirmation_Data_t
{
   unsigned int HCRID;
   unsigned int ConnectionStatus;
} HCR_Notification_Connect_Confirmation_Data_t;

#define HCR_NOTIFICATION_CONNECT_CONFIRMATION_DATA_SIZE (sizeof(HCR_Notification_Connect_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client grants Credits.  The HCRID member   */
   /* specifies the Local HCR Server Connection, the CreditGranted      */
   /* member specifies the Credit value (in Bytes), and the             */
   /* TotalSendCredit member specifies the sum total of all Credits in  */
   /* Bytes) that have been granted to the Server (for Transmission).   */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Grant_Indication_Data_t
{
   unsigned int HCRID;
   DWord_t      CreditGranted;
   DWord_t      TotalSendCredit;
} HCR_Credit_Grant_Indication_Data_t;

#define HCR_CREDIT_GRANT_INDICATION_DATA_SIZE           (sizeof(HCR_Credit_Grant_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Granting of Credits.  */
   /* The HCRID member specifies the Local HCR Client Connection.  The  */
   /* Status parameter specifies the Status (see Status Codes above) of */
   /* the Request.  The TotalReceiveCredit member specifies how much    */
   /* total Credit has been granted to the Server.                      */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Grant_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   DWord_t      TotalReceiveCredit;
} HCR_Credit_Grant_Confirmation_Data_t;

#define HCR_CREDIT_GRANT_CONFIRMATION_DATA_SIZE         (sizeof(HCR_Credit_Grant_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting more Credit to be     */
   /* issued.  The HCRID member specifies the Local HCR Server          */
   /* Connection.  The TotalReceiveCredit member specifies how much     */
   /* Credit has been granted to the Client already (via previous Credit*/
   /* Requests) for transmission to the Server.                         */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Request_Indication_Data_t
{
   unsigned int HCRID;
   DWord_t      TotalReceiveCredit;
} HCR_Credit_Request_Indication_Data_t;

#define HCR_CREDIT_REQUEST_INDICATION_DATA_SIZE         (sizeof(HCR_Credit_Request_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request for more      */
   /* Credit.  The HCRID member specifies the Local HCR Client          */
   /* Connection.  The Status parameter specifies the Status (see Status*/
   /* Codes above) of the Request.  The CreditGranted member specifies  */
   /* the Credit (in Bytes) that was granted to the Client (for         */
   /* Transmission), and the TotalSendCredit member specifies the Total */
   /* Number of Bytes that have been granted (cumulative) to the Client */
   /* for Transmission to the Server.                                   */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Request_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   DWord_t      CreditGranted;
   DWord_t      TotalSendCredit;
} HCR_Credit_Request_Confirmation_Data_t;

#define HCR_CREDIT_REQUEST_CONFIRMATION_DATA_SIZE       (sizeof(HCR_Credit_Request_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is returning Credits that have been */
   /* granted to it previously.  The HCRID member specifies the Local   */
   /* HCR Server Connection, the CreditReturned member specifies the    */
   /* amount of Credit that is being returned (in Bytes), and the       */
   /* TotalReceiveCredit member specifies the total number of Credit    */
   /* that the Client still maintains (after the return, in Bytes).     */
   /* The TotalSendCredit member specifies the total number of Credit   */
   /* (for transmission) that the server maintains (in Bytes).          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Return_Indication_Data_t
{
   unsigned int HCRID;
   DWord_t      CreditReturned;
   DWord_t      TotalReceiveCredit;
   DWord_t      TotalSendCredit;
} HCR_Credit_Return_Indication_Data_t;

#define HCR_CREDIT_RETURN_INDICATION_DATA_SIZE          (sizeof(HCR_Credit_Return_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when     */
   /* the Remotely connected HCR Server processes the Request to        */
   /* Return Credit.  The HCRID member specifies the Local HCR Client   */
   /* Connection, the ReceiveCreditReturned member specifies the Receive*/
   /* Credit (in Bytes) that is being returned to the Client (for       */
   /* Transmission), and the TotalSendCredit member specifies the Total */
   /* Number of Bytes that have been granted (cumulative) to the Client */
   /* (after the Return).  The SendCreditReturned member specifies the  */
   /* Total Number of Bytes that were returned to the server in the The */
   /* Status parameter specifies the Status (see Status Codes above) of */
   /* the Request.                                                      */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Return_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   DWord_t      CreditReturned;
   DWord_t      TotalReceiveCredit;
   DWord_t      TotalSendCredit;
} HCR_Credit_Return_Confirmation_Data_t;

#define HCR_CREDIT_RETURN_CONFIRMATION_DATA_SIZE        (sizeof(HCR_Credit_Return_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting the total amount of   */
   /* Credit (that has been granted to the HCR Server) to be returned.  */
   /* The HCRID member specifies the Local HCR Server Connection and the*/
   /* QueryReceiveCredit member represents the total amount of Credit   */
   /* that the Client believes it has been Granted (in Bytes).  The     */
   /* TotalReceiveCredit member specifies the amount of Credit (in      */
   /* Bytes) that the Server has been granted by the Client (kept track */
   /* of by the Server).                                                */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Query_Indication_Data_t
{
   unsigned int HCRID;
   DWord_t      QueryReceiveCredit;
   DWord_t      TotalReceiveCredit;
} HCR_Credit_Query_Indication_Data_t;

#define HCR_CREDIT_QUERY_INDICATION_DATA_SIZE           (sizeof(HCR_Credit_Query_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to return the */
   /* total amount of granted Credit.  The HCRID member specifies the   */
   /* Local HCR Client Connection and the QueryReceiveCredit member     */
   /* represents the amount of Credit that the Server believes it has   */
   /* been Granted (from the Client, in Bytes).  The TotalReceiveCredit */
   /* member specifies the number of bytes of Credit that has been      */
   /* granted by the Server to the Client (kept track of by the Client).*/
   /* The Status parameter specifies the Status (see Status Codes above)*/
   /* of the Request.                                                   */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Credit_Query_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   DWord_t      QueryReceiveCredit;
   DWord_t      TotalReceiveCredit;
} HCR_Credit_Query_Confirmation_Data_t;

#define HCR_CREDIT_QUERY_CONFIRMATION_DATA_SIZE         (sizeof(HCR_Credit_Query_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting the current LPT       */
   /* Status (IEEE 1284 Job Status) to be returned.  The HCRID member   */
   /* specifies the Local HCR Server Connection that the HCR Client is  */
   /* requesting the LPT Status for.                                    */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_LPT_Status_Query_Indication_Data_t
{
   unsigned int HCRID;
} HCR_LPT_Status_Query_Indication_Data_t;

#define HCR_LPT_STATUS_QUERY_INDICATION_DATA_SIZE       (sizeof(HCR_LPT_Status_Query_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to return the */
   /* current LPT Status (IEEE 1284 Job Status).  The HCRID member      */
   /* specifies the Local HCR Client Connection and the LPTStatus       */
   /* member specifies the IEEE 1284 Job Status Bits (see above).  The  */
   /* Status parameter specifies the Status (see Status Codes above) of */
   /* the Request.                                                      */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_LPT_Status_Query_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   Byte_t       LPTStatus;
} HCR_LPT_Status_Query_Confirmation_Data_t;

#define HCR_LPT_STATUS_QUERY_CONFIRMATION_DATA_SIZE     (sizeof(HCR_LPT_Status_Query_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting the IEEE 1284 ID to be*/
   /* returned.  The HCRID member specifies the Local HCR Server        */
   /* Connection that the HCR Client is requesting the IEEE 1284 ID of. */
   /* The StartingIndex member represents the offset (zero based) into  */
   /* the IEEE 1284 ID String that is to be returned.  The              */
   /* RequestedNumberBytes member specifies the total number of Bytes   */
   /* that are being requested.  The MaximumNumberBytes member is an    */
   /* informational parameter that represents the largest amount of data*/
   /* (in Bytes) that can be transmitted (in a single transaction) to   */
   /* the HCR Client (this value is based upon the Maximum Transmission */
   /* Unit that was negotiated for the Control Channel (L2CAP).         */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_IEEE_1284_ID_Query_Indication_Data_t
{
   unsigned int HCRID;
   Word_t       StartingIndex;
   Word_t       RequestedNumberBytes;
   Word_t       MaximumNumberBytes;
} HCR_IEEE_1284_ID_Query_Indication_Data_t;

#define HCR_IEEE_1284_ID_QUERY_INDICATION_DATA_SIZE     (sizeof(HCR_IEEE_1284_ID_Query_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to return the */
   /* IEEE 1284 ID String.  The HCRID member specifies the Local HCR    */
   /* Client Connection, the NumberBytes parameter specifies the number */
   /* of bytes that are pointed to by the next parameter, IEEE1284Data, */
   /* which is a pointer to a buffer of (at least) NumberBytes in size  */
   /* that represents the portion of the IEEE1284 ID String that was    */
   /* returned.  The Status parameter specifies the Status (see Status  */
   /* Codes above) of the Request.                                      */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_IEEE_1284_ID_Query_Confirmation_Data_t
{
   unsigned int  HCRID;
   Word_t        Status;
   Word_t        NumberBytes;
   Byte_t       *IEEE1284Data;
} HCR_IEEE_1284_ID_Query_Confirmation_Data_t;

#define HCR_IEEE_1284_ID_QUERY_CONFIRMATION_DATA_SIZE   (sizeof(HCR_IEEE_1284_ID_Query_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting a Soft Reset to be    */
   /* performed.  The HCRID member specifies the Local HCR Server       */
   /* Connection that the HCR Client is requesting the Soft Reset of.   */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Soft_Reset_Indication_Data_t
{
   unsigned int HCRID;
} HCR_Soft_Reset_Indication_Data_t;

#define HCR_SOFT_RESET_INDICATION_DATA_SIZE             (sizeof(HCR_Soft_Reset_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to issue a    */
   /* Soft Reset.  The HCRID member specifies the Local HCR Client      */
   /* Connection that the Soft Reset took place on.  The Status         */
   /* parameter specifies the Status (see Status Codes above) of the    */
   /* Request.                                                          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Soft_Reset_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
} HCR_Soft_Reset_Confirmation_Data_t;

#define HCR_SOFT_RESET_CONFIRMATION_DATA_SIZE           (sizeof(HCR_Soft_Reset_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting a Hard Reset to be    */
   /* performed.  The HCRID member specifies the Local HCR Server       */
   /* Connection that the HCR Client is requesting the Hard Reset of.   */
   /* * NOTE * A Hard Reset requires all Channels to be closed.  After  */
   /*          a Hard Reset Event is received, ALL Channels will be     */
   /*          closed.                                                  */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Hard_Reset_Indication_Data_t
{
   unsigned int HCRID;
} HCR_Hard_Reset_Indication_Data_t;

#define HCR_HARD_RESET_INDICATION_DATA_SIZE             (sizeof(HCR_Hard_Reset_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to issue a    */
   /* Hard Reset.  The HCRID member specifies the Local HCR Client      */
   /* Connection that the Hard Reset took place on.  The Status         */
   /* parameter specifies the Status (see Status Codes above) of the    */
   /* Request.                                                          */
   /* * NOTE * A Hard Reset requires all Channels to be closed.  After  */
   /*          a Hard Reset Event is received, ALL Channels will be     */
   /*          closed.                                                  */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Hard_Reset_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
} HCR_Hard_Reset_Confirmation_Data_t;

#define HCR_HARD_RESET_CONFIRMATION_DATA_SIZE           (sizeof(HCR_Hard_Reset_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting a Callback            */
   /* Notification to be Registered.  The HCRID member specifies the    */
   /* Local HCR Server Connection that the HCR Client is requesting the */
   /* Notification Registration.  The Register member specifies whether */
   /* the specified Callback is being Registered (TRUE) or Un-Registered*/
   /* (FALSE).  The CallbackContextID member is an Application Defined  */
   /* value that is to be passed to the Notification Callback when the  */
   /* Callback is performed.  The CallbackTimeout value is the Timeout  */
   /* value (in Milliseconds) that the Client is requesting that the    */
   /* Server keep the Notification Channel Active once the Callback     */
   /* Notification Channel takes place.                                 */
   /* * NOTE * When Registering/Un-Registering a Notification Callback  */
   /*          the Callback Context ID is used to Identify the Callback */
   /*          Registration that is being Registered/Un-Registered.     */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Register_Notification_Indication_Data_t
{
   unsigned int HCRID;
   Boolean_t    Register;
   DWord_t      CallbackContextID;
   DWord_t      CallbackTimeout;
} HCR_Register_Notification_Indication_Data_t;

#define HCR_REGISTER_NOTIFICATION_INDICATION_DATA_SIZE  (sizeof(HCR_Register_Notification_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to Register a */
   /* Notification Callback.  The HCRID member specifies the Local HCR  */
   /* Client Connection that the Notification was registered for.  The  */
   /* NotificationTimeout member specifies the amount of Time (in       */
   /* Milliseconds) that will pass before the Notification will expire  */
   /* (i.e. will no longer be called).  The CallbackTimeout member      */
   /* specifies the amount of Time (in Milliseconds) that the Server    */
   /* will keep the Notification Channel Open after the Notification    */
   /* Channel is connected.  The Status parameter specifies the Status  */
   /* (see Status Codes above) of the Request.                          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Register_Notification_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   DWord_t      NotificationTimeout;
   DWord_t      CallbackTimeout;
} HCR_Register_Notification_Confirmation_Data_t;

#define HCR_REGISTER_NOTIFICATION_CONFIRMATION_DATA_SIZE (sizeof(HCR_Register_Notification_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Server when the */
   /* Remotely connected HCR Client is requesting that the Server extend*/
   /* the amount of time that the Notification Channel is Connected.    */
   /* The HCRID member specifies the Local HCR Server Connection that   */
   /* the HCR Client is requesting the Notification Timeout Extension.  */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Notification_Connection_Alive_Indication_Data_t
{
   unsigned int HCRID;
} HCR_Notification_Connection_Alive_Indication_Data_t;

#define HCR_NOTIFICATION_CONNECTION_ALIVE_INDICATION_DATA_SIZE (sizeof(HCR_Notification_Connection_Alive_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Client when the */
   /* Remotely connected HCR Server processes the Request to extend the */
   /* Time that the Notification Channel is active before it is         */
   /* disconnected.  The HCRID member specifies the Local HCR Client    */
   /* Connection that the Notification is being extended for.  The      */
   /* NotificationTimeout member specifies the amount of Time (in       */
   /* Milliseconds) that the current Notification Timeout will be       */
   /* extended (i.e. will be extended before the Server disconnects the */
   /* Notification Channel).  The Status parameter specifies the Status */
   /* (see Status Codes above) of the Request.                          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
typedef struct _tagHCR_Notification_Connection_Alive_Confirmation_Data_t
{
   unsigned int HCRID;
   Word_t       Status;
   DWord_t      NotificationTimeout;
} HCR_Notification_Connection_Alive_Confirmation_Data_t;

#define HCR_NOTIFICATION_CONNECTION_ALIVE_CONFIRMATION_DATA_SIZE (sizeof(HCR_Notification_Connection_Alive_Confirmation_Data_t))

   /* The following HCR Event is dispatched to a an HCR Profile when the*/
   /* Remotely connected HCR Profile sends Vendor Specific Data.  In the*/
   /* case of this information being sent on the Control Channel, the   */
   /* Data will be from the Client to the Server (the Server will       */
   /* receive this Event).  In the case where this is received on the   */
   /* Notification Channel (both the Client and the Server can receive  */
   /* this Message).  The HCRID member specifies the Local HCR          */
   /* Connection that has received the data, the DataLength member      */
   /* specifies the Amount of Data that was received (in Bytes), and the*/
   /* DataBuffer member is a pointer to a buffer that contains (at      */
   /* least) the number of Bytes of Data that was received.             */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
   /* * NOTE * This Event can also be dispatched on the Notification    */
   /*          Channel (it appears from the Specification).  In this    */
   /*          case it is allowed that only ONE Vendor Specific Message */
   /*          can be outstanding at a time (for a given direction -    */
   /*          either from Client to Server or Server to Client).       */
typedef struct _tagHCR_Vendor_Specific_Indication_Data_t
{
   unsigned int  HCRID;
   Word_t        PDUID;
   unsigned int  DataLength;
   Byte_t       *DataBuffer;
} HCR_Vendor_Specific_Indication_Data_t;

#define HCR_VENDOR_SPECIFIC_INDICATION_DATA_SIZE        (sizeof(HCR_Vendor_Specific_Indication_Data_t))

   /* The following HCR Event is dispatched to a an HCR Profile when the*/
   /* Remotely connected HCR Profile responds to previously sent Vendor */
   /* Specific Data.  In the case of this information being sent on the */
   /* Control Channel, the Data will be from the Server to the Client   */
   /* (the Client will receive this Event).  In the case where this is  */
   /* received on the Notification Channel (both the Client and the     */
   /* Server can receive this Message).  The HCRID member specifies the */
   /* Local HCR Connection that has received the data, the DataLength   */
   /* member specifies the Amount of Data that was received (in Bytes), */
   /* and the DataBuffer member is a pointer to a buffer that contains  */
   /* (at least) the number of Bytes of Data that was received.  The    */
   /* Status parameter specifies the Status (see Status Codes above) of */
   /* the Request.                                                      */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, any Control Action  */
   /*          that is originated (by the Client) MUST wait for a       */
   /*          response before another action is initiated.             */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
   /* * NOTE * This Event can also be dispatched on the Notification    */
   /*          Channel (it appears from the Specification).  In this    */
   /*          case it is allowed that only ONE Vendor Specific Message */
   /*          can be outstanding at a time (for a given direction -    */
   /*          either from Client to Server or Server to Client).       */
typedef struct _tagHCR_Vendor_Specific_Confirmation_Data_t
{
   unsigned int  HCRID;
   Word_t        PDUID;
   Word_t        Status;
   unsigned int  DataLength;
   Byte_t       *DataBuffer;
} HCR_Vendor_Specific_Confirmation_Data_t;

#define HCR_VENDOR_SPECIFIC_CONFIRMATION_DATA_SIZE      (sizeof(HCR_Vendor_Specific_Confirmation_Data_t))

   /* The following HCR Event is dispatched when data is received from a*/
   /* remote HCR Unit (Client or Server).  The HCRID member specifies   */
   /* the Local HCR Connection that has received the data, the          */
   /* DataLength member specifies the Amount of Data that was received  */
   /* (in Bytes), and the DataBuffer member is a pointer to a buffer    */
   /* that contains (at least) the number of Bytes of Data that was     */
   /* received.                                                         */
typedef struct _tagHCR_Data_Indication_Data_t
{
   unsigned int  HCRID;
   unsigned int  DataLength;
   Byte_t       *DataBuffer;
} HCR_Data_Indication_Data_t;

#define HCR_DATA_INDICATION_DATA_SIZE                   (sizeof(HCR_Data_Indication_Data_t))

   /* The following HCR Event is dispatched when data is received from a*/
   /* remote HCR Server on the Notification Channel.  The HCRID member  */
   /* specifies the Local HCR Connection that has received the          */
   /* Notification and the CallbackContextID parameter specifies the    */
   /* Callback Context Information that was registered with the Server  */
   /* when the Notification was registered.  Upon Receiving this        */
   /* Notification the Notification Channel is immediately closed so no */
   /* further communication can occur on this Channel.                  */
typedef struct _tagHCR_Notification_Indication_Data_t
{
   unsigned int HCRID;
   DWord_t      CallbackContextID;
} HCR_Notification_Indication_Data_t;

#define HCR_NOTIFICATION_INDICATION_DATA_SIZE           (sizeof(HCR_Notification_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all HCR Profile Event Data Data.                          */
typedef struct _tagHCR_Event_Data_t
{
   HCR_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      HCR_Connect_Request_Indication_Data_t                 *HCR_Connect_Request_Indication_Data;
      HCR_Control_Connect_Indication_Data_t                 *HCR_Control_Connect_Indication_Data;
      HCR_Data_Connect_Indication_Data_t                    *HCR_Data_Connect_Indication_Data;
      HCR_Notification_Connect_Indication_Data_t            *HCR_Notification_Connect_Indication_Data;
      HCR_Control_Connect_Confirmation_Data_t               *HCR_Control_Connect_Confirmation_Data;
      HCR_Data_Connect_Confirmation_Data_t                  *HCR_Data_Connect_Confirmation_Data;
      HCR_Notification_Connect_Confirmation_Data_t          *HCR_Notification_Connect_Confirmation_Data;
      HCR_Control_Disconnect_Indication_Data_t              *HCR_Control_Disconnect_Indication_Data;
      HCR_Data_Disconnect_Indication_Data_t                 *HCR_Data_Disconnect_Indication_Data;
      HCR_Notification_Disconnect_Indication_Data_t         *HCR_Notification_Disconnect_Indication_Data;
      HCR_Credit_Grant_Indication_Data_t                    *HCR_Credit_Grant_Indication_Data;
      HCR_Credit_Grant_Confirmation_Data_t                  *HCR_Credit_Grant_Confirmation_Data;
      HCR_Credit_Request_Indication_Data_t                  *HCR_Credit_Request_Indication_Data;
      HCR_Credit_Request_Confirmation_Data_t                *HCR_Credit_Request_Confirmation_Data;
      HCR_Credit_Return_Indication_Data_t                   *HCR_Credit_Return_Indication_Data;
      HCR_Credit_Return_Confirmation_Data_t                 *HCR_Credit_Return_Confirmation_Data;
      HCR_Credit_Query_Indication_Data_t                    *HCR_Credit_Query_Indication_Data;
      HCR_Credit_Query_Confirmation_Data_t                  *HCR_Credit_Query_Confirmation_Data;
      HCR_LPT_Status_Query_Indication_Data_t                *HCR_LPT_Status_Query_Indication_Data;
      HCR_LPT_Status_Query_Confirmation_Data_t              *HCR_LPT_Status_Query_Confirmation_Data;
      HCR_IEEE_1284_ID_Query_Indication_Data_t              *HCR_IEEE_1284_ID_Query_Indication_Data;
      HCR_IEEE_1284_ID_Query_Confirmation_Data_t            *HCR_IEEE_1284_ID_Query_Confirmation_Data;
      HCR_Soft_Reset_Indication_Data_t                      *HCR_Soft_Reset_Indication_Data;
      HCR_Soft_Reset_Confirmation_Data_t                    *HCR_Soft_Reset_Confirmation_Data;
      HCR_Hard_Reset_Indication_Data_t                      *HCR_Hard_Reset_Indication_Data;
      HCR_Hard_Reset_Confirmation_Data_t                    *HCR_Hard_Reset_Confirmation_Data;
      HCR_Register_Notification_Indication_Data_t           *HCR_Register_Notification_Indication_Data;
      HCR_Register_Notification_Confirmation_Data_t         *HCR_Register_Notification_Confirmation_Data;
      HCR_Notification_Connection_Alive_Indication_Data_t   *HCR_Notification_Connection_Alive_Indication_Data;
      HCR_Notification_Connection_Alive_Confirmation_Data_t *HCR_Notification_Connection_Alive_Confirmation_Data;
      HCR_Vendor_Specific_Indication_Data_t                 *HCR_Vendor_Specific_Indication_Data;
      HCR_Vendor_Specific_Confirmation_Data_t               *HCR_Vendor_Specific_Confirmation_Data;
      HCR_Data_Indication_Data_t                            *HCR_Data_Indication_Data;
      HCR_Notification_Indication_Data_t                    *HCR_Notification_Indication_Data;
   } Event_Data;
} HCR_Event_Data_t;

#define HCR_EVENT_DATA_SIZE                             (sizeof(HCR_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a HCR Profile Event Receive Data Callback.  This function will be */
   /* called whenever a HCR Event occurs that is associated with the    */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the HCR Event Data that occurred and the  */
   /* HCR Event Callback Parameter that was specified when this         */
   /* Callback was installed.  The caller is free to use the contents   */
   /* of the HCR Event Data ONLY in the context of this callback.  If   */
   /* the caller requires the Data for a longer period of time, then the*/
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Hard Copy Cable       */
   /* Replacement Profile Event will not be processed while this        */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving HCR Events.  A      */
   /*            Deadlock WILL occur because NO HCR Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *HCR_Event_Callback_t)(unsigned int BluetoothStackID, HCR_Event_Data_t *HCR_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a HCR Server on */
   /* the specified Bluetooth L2CAP Control and Data Channels.  This    */
   /* function accepts as input the Bluetooth Stack ID of the Bluetooth */
   /* Stack Instance to use for the HCR Server, the Local Control Server*/
   /* Channel to use, the Local Data Server Channel to use, the type of */
   /* Service that this HCR Server will service, and the HCR Server     */
   /* Event Callback function (and parameter) to associate with the     */
   /* specified HCR Server.  The Server Channel parameters *MUST* be    */
   /* between L2CAP_PSM_MINIMUM_PSM and L2CAP_PSM_MAXIMUM_PSM and each  */
   /* of the Server Channels must be unique.  This function returns a   */
   /* positive, non zero, value if successful or a negative return      */
   /* error code if an error occurs.  A successful return code will be a*/
   /* HCR ID that can be used to reference the Opened HCR Server in ALL */
   /* other HCR functions in this module.  Once a HCR Server is opened, */
   /* it can only be Un-Registered via a call to the HCR_Close_Server() */
   /* function (passing the return value from this function).           */
BTPSAPI_DECLARATION int BTPSAPI HCR_Open_Server(unsigned int BluetoothStackID, unsigned int ControlServerChannel, unsigned int DataServerChannel, HCRServiceType_t ServiceType, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Open_Server_t)(unsigned int BluetoothStackID, unsigned int ControlServerChannel, unsigned int DataServerChannel, HCRServiceType_t ServiceType, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Opening a HCR           */
   /* Notification Server on the specified Bluetooth L2CAP Channel.     */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Stack Instance to use for the HCR Notification Server,  */
   /* the Local L2CAP Channel to use, the type of Service that this HCR */
   /* Notification Server will service, and the HCR Server Event        */
   /* Callback function (and parameter) to associate with the specified */
   /* Notification Server.  The Server Channel parameters *MUST* be     */
   /* between L2CAP_PSM_MINIMUM_PSM and L2CAP_PSM_MAXIMUM_PSM.  This    */
   /* function returns a positive, non zero, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a HCR ID that can be used to reference the    */
   /* Opened HCR Notification Server in ALL other HCR functions in this */
   /* module.  Once a HCR Notification Server is opened, it can only be */
   /* Un-Registered via a call to the HCR_Close_Server() function       */
   /* (passing the return value from this function).                    */
BTPSAPI_DECLARATION int BTPSAPI HCR_Open_Notification_Server(unsigned int BluetoothStackID, unsigned int ServerChannel, HCRServiceType_t ServiceType, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Open_Notification_Server_t)(unsigned int BluetoothStackID, unsigned int ServerChannel, HCRServiceType_t ServiceType, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local HCR Server.  The first   */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Stack associated with the HCR Server that is responding */
   /* to the request.  The second parameter to this function is the HCR */
   /* ID of the HCR Server for which an connect request was received.   */
   /* The final parameter to this function specifies whether to accept  */
   /* the pending connection request (or to reject the request).  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
   /* ** NOTE ** The connection to the server is not established until  */
   /*            either:                                                */
   /*               etHCR_Control_Connect_Confirmation                  */
   /*               etHCR_Data_Connect_Indication                       */
   /*               etHCR_Notification_Connect_Indication               */
   /*            event has occurred (depending upon the type of         */
   /*            server connection).                                    */
   /* ** NOTE ** This event will only be dispatched if the server mode  */
   /*            was explicitly set to csmManualAccept via the          */
   /*            HCR_Set_Server_Connection_Mode() function.             */
BTPSAPI_DECLARATION int BTPSAPI HCR_Connect_Request_Response(unsigned int BluetoothStackID, unsigned int HCRID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Connect_Request_Response_t)(unsigned int BluetoothStackID, unsigned int HCRID, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for Un-Registering a HCR    */
   /* Server (which was Registered by a successful call to the          */
   /* HCR_Open_Server() or HCR_Open_Notification_Server() functions).   */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the HCR Server specified by the     */
   /* Second Parameter is valid for.  This function returns zero if     */
   /* successful, or a negative return error code if an error occurred  */
   /* (see BTERRORS.H).  Note that this function does NOT delete any SDP*/
   /* Service Record Handles.                                           */
BTPSAPI_DECLARATION int BTPSAPI HCR_Close_Server(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Close_Server_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a means to add a Hard */
   /* Copy Cable Replacement Server Service Record to the SDP Database. */
   /* This function takes as input the Bluetooth Stack ID of the Local  */
   /* Bluetooth Protocol Stack, the Hard Copy Cable Replacement Profile */
   /* ID (which *MUST* have been obtained by calling the                */
   /* HCR_Open_Server() function).  The third parameter (if NON-NULL    */
   /* 16, 32, or 128 Bit UUID) specifies the Optional Service ID to     */
   /* add to the Service Class ID List.  The fourth parameter specifies */
   /* the Service Name to associate with the SDP Record.  The fifth     */
   /* parameter (if specified) specifies the IEEE 1284ID String to add  */
   /* the Record (this parameter is optional).  The next three          */
   /* parameters are optional and they specify the Device Name, Friendly*/
   /* Name, and Device Location strings to add to the Record.  The final*/
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the HCRID that  */
   /*          was returned from the HCR_Open_Server() function.  This  */
   /*          function should NEVER be used with the HCRID returned    */
   /*          from the HCR_Open_Notification_Server() function.        */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          HCR_Un_Register_SDP_Record() to                          */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
   /* * NOTE * All Text Strings (Service Name, Device Name, Friendly    */
   /*          Name, and Device Location) are NULL terminated ASCII     */
   /*          strings that have maximum lengths given by the constants:*/
   /*             - HCR_SERVICE_NAME_MAXIMUM_LENGTH                     */
   /*             - HCR_DEVICE_NAME_MAXIMUM_LENGTH                      */
   /*             - HCR_FRIENDLY_NAME_MAXIMUM_LENGTH                    */
   /*             - HCR_DEVICE_LOCATION_NAME_MAXIMUM_LENGTH             */
BTPSAPI_DECLARATION int BTPSAPI HCR_Register_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int HCRID, SDP_UUID_Entry_t *ServiceID, char *ServiceName, HCR_SDP_1284_ID_Information_t *HCR_SDP_1284_ID_Information, char *DeviceName, char *FriendlyName, char *DeviceLocation, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Register_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int HCRID, SDP_UUID_Entry_t *ServiceID, char *ServiceName, HCR_SDP_1284_ID_Information_t *HCR_SDP_1284_ID_Information, char *DeviceName, char *FriendlyName, char *DeviceLocation, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following function is provided to allow a means to add a Hard */
   /* Copy Cable Replacement Notification Server Service Record to the  */
   /* SDP Database.  This function takes as input the Bluetooth Stack ID*/
   /* of the Local Bluetooth Protocol Stack, the Hard Copy Cable        */
   /* Replacement Notification ID (which *MUST* have been obtained by   */
   /* calling the HCR_Open_Notification_Server() function).  The next   */
   /* parameter is the Service Name to associate with the SDP Record.   */
   /* The final parameter is a pointer to a DWord_t which receives the  */
   /* SDP Service Record Handle if this function successfully creates an*/
   /* SDP Service Record.  If this function returns zero, then the      */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the HCRID that  */
   /*          was returned from the HCR_Open_Notification_Server()     */
   /*          function.  This function should NEVER be used with the   */
   /*          HCRID returned from the HCR_Open_Server() function.      */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          HCR_Un_Register_SDP_Record() to                          */
   /*          SDP_Delete_Service_Record().                             */
BTPSAPI_DECLARATION int BTPSAPI HCR_Register_Notification_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int HCRID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Register_Notification_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int HCRID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* HCR Server SDP Service Record (specified by the third parameter)  */
   /* from the SDP Database.  This MACRO simply maps to the             */
   /* SDP_Delete_Service_Record() function.  This MACRO is only         */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the HCR Server ID (returned from    */
   /* a successful call to the HCR_Open_Server() function), and the SDP */
   /* Service Record Handle.  The SDP Service Record Handle was         */
   /* returned via a successful call to the                             */
   /* HCR_Register_Server_SDP_Record() or the                           */
   /* HCR_Register_Notification_Server_SDP_Record() function.  See the  */
   /* HCR_Register_Server_SDP_Record() or the                           */
   /* HCR_Register_Notification_Server_SDP_Record() function for more   */
   /* information.  This MACRO returns the result of the                */
   /* SDP_Delete_Service_Record() function, which is zero for success   */
   /* or a negative return error code (see BTERRORS.H).                 */
#define HCR_Un_Register_SDP_Record(__BluetoothStackID, __HCRID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for opening a Remote Hard   */
   /* Copy Cable Replacement Server.  This function accepts as input the*/
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the HCR   */
   /* Client is associated with.  The second parameter is the Remote    */
   /* Bluetooth Device Address of the Bluetooth HCR Server to connect   */
   /* with.  The third parameter specifies the Remote Control L2CAP     */
   /* Channel to connect with.  The final two parameters specify the HCR*/
   /* Event Callback Function and the Callback Parameter to associate   */
   /* with this HCR Client.  The Server Channel parameter *MUST* be     */
   /* between L2CAP_PSM_MINIMUM_PSM and L2CAP_PSM_MAXIMUM_PSM.  This    */
   /* function returns a positive, non zero, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a HCR ID that can be used to reference the    */
   /* Opened HCR Server in ALL other HCR Client functions in this       */
   /* module.  Once a remote server is opened, it can only be closed via*/
   /* a call to the HCR_Close_Client() function (passing the return     */
   /* value from this function).                                        */
BTPSAPI_DECLARATION int BTPSAPI HCR_Open_Remote_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ControlServerChannel, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Open_Remote_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ControlServerChannel, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a Data Channel  */
   /* on a currently connected Copy Cable Replacement Server (i.e the   */
   /* Control Channel has already been established).  This function     */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack that the HCR Client is associated with.  The second         */
   /* parameter is the HCR Client ID (returned via a successful call to */
   /* the HCR_Open_Remote_Server() function) of the HCR Client to       */
   /* establish a Data Channel with.  The final parameter to this       */
   /* function is the L2CAP Channel of the Data Channel that is located */
   /* on the Remote HCR Server.  The Server Channel parameter *MUST* be */
   /* between L2CAP_PSM_MINIMUM_PSM and L2CAP_PSM_MAXIMUM_PSM.  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if an error occurs.  This function can only be called AFTER a*/
   /* successful Connection Indication has been received for the Control*/
   /* Channel.  The caller will be notified via the Callback that was   */
   /* installed via the HCR_Open_Remote_Server() function when the Data */
   /* Channel is fully established.  After the Data Channel is          */
   /* established, the caller is free to start sending data (given that */
   /* Credit has been granted).                                         */
BTPSAPI_DECLARATION int BTPSAPI HCR_Open_Data_Channel(unsigned int BluetoothStackID, unsigned int HCRID, unsigned int DataServerChannel);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Open_Data_Channel_t)(unsigned int BluetoothStackID, unsigned int HCRID, unsigned int DataServerChannel);
#endif

   /* The following function is responsible for opening a Remote Hard   */
   /* Copy Cable Replacement Notification Server.  This function accepts*/
   /* as input the Bluetooth Stack ID of the Bluetooth Protocol Stack   */
   /* that the HCR Notification Client is associated with.  The second  */
   /* parameter is the Remote Bluetooth Device Address of the Bluetooth */
   /* HCR Notification Server to connect with.  The third parameter     */
   /* specifies the Remote L2CAP Channel to connect with.  The final two*/
   /* parameters specify the HCR Event Callback Function and the        */
   /* Callback Parameter to associate with this HCR Client.  The Server */
   /* Channel parameter *MUST* be between L2CAP_PSM_MINIMUM_PSM and     */
   /* L2CAP_PSM_MAXIMUM_PSM.  This function returns a positive, non     */
   /* zero, value if successful or a negative return error code if an   */
   /* error occurs.  A successful return code will be a HCR ID that can */
   /* be used to reference the Opened HCR Server in ALL other HCR Client*/
   /* functions in this module.  Once a Remote Notification Server is   */
   /* opened, it can only be closed via a call to the HCR_Close_Client()*/
   /* function (passing the return value from this function).           */
BTPSAPI_DECLARATION int BTPSAPI HCR_Open_Remote_Notification_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerChannel, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Open_Remote_Notification_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerChannel, HCR_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function exists to close a Hard Copy Cable          */
   /* Replacement Profile that was previously opened with the           */
   /* HCR_Open_Remote_Server() function OR the                          */
   /* HCR_Open_Remote_Notification_Server() function.  This function    */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which the Open HCR Profile resides and the HCR ID (return value   */
   /* from one of the above mentioned Open functions) of the Client to  */
   /* Close.  This function returns zero if successful, or a negative   */
   /* return value if there was an error.  This function does NOT       */
   /* Un-Register a HCR Server from the system, it ONLY disconnects any */
   /* connection that is currently active on the Server Port.  The      */
   /* HCR_Close_Server() function can be used to Un-Register the HCR    */
   /* Server or the HCR Notification Server .                           */
BTPSAPI_DECLARATION int BTPSAPI HCR_Close(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Close_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to grant credit to the Remote HCR Server.    */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the HCR Client ID is valid with     */
   /* (specified by the second parameter).  The final parameter         */
   /* specifies the total number of bytes of Credit to grant to the     */
   /* remote HCR Server.  This function returns zero if successful, or a*/
   /* negative return value if there was an error.                      */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Grant_Request(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t CreditGrant);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Grant_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t CreditGrant);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a credit grant from a Remote   */
   /* HCR Client.  This function accepts as input the Bluetooth Stack   */
   /* ID of the Bluetooth Stack the HCR Server ID is valid with         */
   /* (specified by the second parameter).  The final parameter         */
   /* specifies the PDU Status value to return.  Valid values are:      */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Grant_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Grant_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to request a grant of credit from the Remote */
   /* HCR Server.  This function accepts as input the Bluetooth Stack ID*/
   /* of the Bluetooth Protocol Stack that the HCR Client ID is valid   */
   /* with (specified by the second parameter).  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Request_Request(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Request_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a request for credit grant from*/
   /* a Remote HCR Client.  This function accepts as input the Bluetooth*/
   /* Stack ID of the Bluetooth Stack the HCR Server ID is valid with   */
   /* (specified by the second parameter).  The third parameter to this */
   /* function specifies the PDU Status value to return to return.      */
   /* Valid values are:                                                 */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* The final parameter to this function is the number of bytes of    */
   /* credit to grant to the remote HCR Client.  Note that this         */
   /* parameter is ignored if the Reply Status Code is anything other   */
   /* that Success.  This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Request_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t CreditGrant);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Request_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t CreditGrant);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to return any unused credit back to the      */
   /* Remote HCR Server.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Protocol Stack that the HCR Client ID is*/
   /* valid with (specified by the second parameter).  The final        */
   /* parameter to this function is the number of bytes of credit that  */
   /* the HCR Client is returning to the HCR Server.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Return_Request(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t CreditReturn);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Return_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t CreditReturn);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a credit return from a Remote  */
   /* HCR Client.  This function accepts as input the Bluetooth Stack ID*/
   /* of the Bluetooth Stack the HCR Server ID is valid with (specified */
   /* by the second parameter).  The final parameter to this function is*/
   /* by the second parameter).  The third parameter to this function   */
   /* specifies the PDU Status value to return to return.  Valid values */
   /* are:                                                              */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* The final parameter to this function is the number of bytes of of */
   /* credit (if any) to return back to the remote HCR Client.  This    */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Return_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t CreditReturn);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Return_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t CreditReturn);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to query the amount of credit available from */
   /* the Remote HCR Server.  This function accepts as input the        */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the HCR   */
   /* Client ID is valid with (specified by the second parameter).  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* * NOTE * Since the Hard Copy Cable Replacement Profile Module     */
   /*          keeps track of all credit internally, the caller doesn't */
   /*          have to specify the amount of Credit that the HCR Client */
   /*          thinks the remote HCR Server has (this will be taken care*/
   /*          of internally).                                          */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Query_Request(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Query_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a credit query from a Remote   */
   /* HCR Client.  This function accepts as input the Bluetooth Stack ID*/
   /* of the Bluetooth Stack the HCR Server ID is valid with (specified */
   /* by the second parameter).  The third parameter specifies the PDU  */
   /* Status value to return to return.  Valid values are:              */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * Since the Hard Copy Cable Replacement Profile Module     */
   /*          keeps track of all credit internally, the caller doesn't */
   /*          have to specify the amount of Credit that the HCR Server */
   /*          thinks the remote HCR Client has (this will be taken care*/
   /*          of internally).                                          */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Credit_Query_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Credit_Query_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to query the current LPT Job Status from the */
   /* Remote HCR Server.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Protocol Stack that the HCR Client ID is*/
   /* valid with (specified by the second parameter).  This function    */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_LPT_Status_Query_Request(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_LPT_Status_Query_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to an LPT Job Status query from a */
   /* Remote HCR Client.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Stack the HCR Server ID is valid with   */
   /* (specified by the second parameter).  The third parameter         */
   /* specifies the PDU Status value to return to return.  Valid values */
   /* are:                                                              */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* The final parameter to this function specifies the current LPT Job*/
   /* Status to return to the HCR Client.  Note that this parameter is  */
   /* ignored if the Reply Status Code is anything other that Success.  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_LPT_Status_Query_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, Byte_t LPTStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_LPT_Status_Query_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, Byte_t LPTStatus);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to query the IEEE 1284ID String from the     */
   /* Remote HCR Server.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Protocol Stack that the HCR Client ID is*/
   /* valid with (specified by the second parameter).  The third        */
   /* parameter specifies the Starting Index into the 1284ID String of  */
   /* the first byte to return.  The final parameter specifies the      */
   /* maximum number of bytes (starting from the index in the third     */
   /* parameter) to return of the IEEE 1284ID String.  This function    */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_IEEE_1284_ID_String_Query_Request(unsigned int BluetoothStackID, unsigned int HCRID, Word_t StartingIndex, Word_t NumberBytes);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_IEEE_1284_ID_String_Query_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t StartingIndex, Word_t NumberBytes);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to an IEEE 1284ID String Request  */
   /* from a Remote HCR Client.  This function accepts as input the     */
   /* Bluetooth Stack ID of the Bluetooth Stack the HCR Server ID is    */
   /* valid with (specified by the second parameter).  The third        */
   /* parameter specifies the PDU Status value to return to return.     */
   /* Valid values are:                                                 */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* The fourth parameter to this function specifies the number of     */
   /* bytes of the IEEE 1284ID String are being returned.  The final    */
   /* parameter to this function specifies a buffer that contains (at   */
   /* least) as many bytes as specified by the third parameter which    */
   /* represents the IEEE 1284ID String that is to be returned to the   */
   /* HCR Client.  Note that if the Status Code that is supplied is     */
   /* any value other than success then the final two parameters of     */
   /* this function are ignored.  This function returns the number of   */
   /* bytes that were successfully sent (non-zero) if successful, or a  */
   /* negative return value if there was an error.                      */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
   /* * NOTE * It is possible that a string can be sent that is larger  */
   /*          than the MTU value that was negotiated for the Control   */
   /*          Channel.  Because of this, this function will only send  */
   /*          up to the Maximum Number of Bytes that will fit within   */
   /*          a Control Channel Packet (based upon the negotiated MTU).*/
BTPSAPI_DECLARATION int BTPSAPI HCR_IEEE_1284_ID_String_Query_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, Word_t StringLength, Byte_t *StringData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_IEEE_1284_ID_String_Query_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, Word_t StringLength, Byte_t *StringData);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to request the Remote HCR Server to perform a*/
   /* Soft Reset.  This function accepts as input the Bluetooth Stack   */
   /* ID of the Bluetooth Protocol Stack that the HCR Client ID is      */
   /* valid with (specified by the second parameter).  This function    */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Soft_Reset_Request(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Soft_Reset_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a Soft Reset Request from a    */
   /* Remote HCR Client.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Stack the HCR Server ID is valid with   */
   /* (specified by the second parameter).  The final parameter         */
   /* specifies the PDU Status value to return.  Valid values are:      */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Soft_Reset_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Soft_Reset_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to request the Remote HCR Server to perform a*/
   /* Hard Reset.  This function accepts as input the Bluetooth Stack   */
   /* ID of the Bluetooth Protocol Stack that the HCR Client ID is      */
   /* valid with (specified by the second parameter).  This function    */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* * NOTE * A Hard Reset causes all Control and Data Channels to be  */
   /*          Disconnected.  This will occur after the Reply to the    */
   /*          Hard Reset is received from the HCR Server.              */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Hard_Reset_Request(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Hard_Reset_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a Hard Reset Request from a    */
   /* Remote HCR Client.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Stack the HCR Server ID is valid with   */
   /* (specified by the second parameter).  The final parameter         */
   /* specifies the PDU Status value to return.  Valid values are:      */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * A Hard Reset causes all Control and Data Channels to be  */
   /*          Disconnected.  This will occur after the Reply to the    */
   /*          Hard Reset is sent back to the HCR Client.               */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Hard_Reset_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Hard_Reset_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to register a Notification Callback with a   */
   /* Remote HCR Server.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Protocol Stack that the HCR Client ID is*/
   /* valid with (specified by the second parameter).  The third        */
   /* parameter specifies whether or not the specified Notification     */
   /* Callback (via Callback Context ID) is being Registered (TRUE) or  */
   /* Un-Registered (FALSE).  The fourth parameter is the Callback      */
   /* Context ID value that will used in the Notification Message as    */
   /* well as the value that will Identify this Registration.  The      */
   /* final parameter to this function specifies the Timeout value (in  */
   /* Milliseconds) that the Registration is to remain active on the    */
   /* Server.  After this Timeout elapses, the Registration will be     */
   /* dropped from the Server (the Client will not be notified).  It is */
   /* the Client's responsibility to keep registering with the Server to*/
   /* keep the registration alive.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Register_Notification_Request(unsigned int BluetoothStackID, unsigned int HCRID, Boolean_t Register, DWord_t CallbackContextID, DWord_t CallbackTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Register_Notification_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID, Boolean_t Register, DWord_t CallbackContextID, DWord_t CallbackTimeout);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a Notification Registration    */
   /* Request from a Remote HCR Client.  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Stack the HCR Server ID is*/
   /* valid with (specified by the second parameter).  The third        */
   /* parameter specifies the PDU Status value to return.  Valid values */
   /* are:                                                              */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* The fourth parameter to this function specifies the Timeout (in   */
   /* Milliseconds) that the Server will allow to elapse before the     */
   /* Registration Notification expires.  The final parameter to this   */
   /* function represents the time (in Milliseconds) that the Server    */
   /* will keep the Notification Channel established once a Connection  */
   /* is made (before the channel will be closed).  Note that if the    */
   /* Status Code that is supplied is any value other than success then */
   /* the final two parameters of this function are ignored.  This      */
   /* function returns returns zero if successful, or a negative return */
   /* value if there was an error.                                      */
   /* * NOTE * A Hard Reset causes all Control and Data Channels to be  */
   /*          Disconnected.  This will occur after the Reply to the    */
   /*          Hard Reset is sent back to the HCR Client.               */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Register_Notification_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t NotificationTimeout, DWord_t CallbackTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Register_Notification_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t NotificationTimeout, DWord_t CallbackTimeout);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to request the HCR Server to keep the        */
   /* Notification Channel (which *MUST* be currently connected) open.  */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the HCR Client ID is valid with     */
   /* (specified by the second parameter).  This function returns zero  */
   /* if successful, or a negative return value if there was an error.  */
   /* * NOTE * This function is NOT a request to keep the Notification  */
   /*          Timeout from expiring, but a request to NOT drop the     */
   /*          Notification Connection.  For this command to have any   */
   /*          effect it can only be issued while the Notification      */
   /*          Channel Connection is active.                            */
   /* * NOTE * This function can only be issued by an HCR Client.  HCR  */
   /*          Servers are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Notification_Connection_Alive_Request(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Notification_Connection_Alive_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to a Notification Connection Alive*/
   /* Request from a Remote HCR Client.  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Stack the HCR Server ID is*/
   /* valid with (specified by the second parameter).  The third        */
   /* parameter specifies the PDU Status value to return.  Valid values */
   /* are:                                                              */
   /*    - HCR_PDU_STATUS_FEATURE_UNSUPPORTED                           */
   /*    - HCR_PDU_STATUS_SUCCESS                                       */
   /*    - HCR_PDU_STATUS_CREDIT_SYNCHRONIZATION_ERROR                  */
   /*    - HCR_PDU_STATUS_GENERIC_FAILURE                               */
   /* The final parameter to this function specifies the Timeout (in    */
   /* Milliseconds) that the Server has extended the Notification       */
   /* Connection Timeout.  Note that if the Status Code that is supplied*/
   /* is any value other than Success, the final parameter of this      */
   /* function is ignored.  This function returns zero if successful, or*/
   /* a negative return value if there was an error.                    */
   /* * NOTE * A Hard Reset causes all Control and Data Channels to be  */
   /*          Disconnected.  This will occur after the Reply to the    */
   /*          Hard Reset is sent back to the HCR Client.               */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Notification_Connection_Alive_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t NotificationTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Notification_Connection_Alive_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, DWord_t NotificationTimeout);
#endif

   /* The following function is responsible for Sending Data over the   */
   /* the specified Hard Copy Cable Replacement Connection.  The HCRID  */
   /* that is passed to this function MUST have been established by     */
   /* either Accepting a HCR Connection (callback from the              */
   /* HCR_Open_Server() function) or by initiating an HCR Client        */
   /* Connection (via calling the HCR_Open_Remote_Server() function and */
   /* having the remote side accept the Connection).  A Data Channel    */
   /* *MUST* be established before this function can be called.  The    */
   /* input parameters to this function are the Bluetooth Stack ID of   */
   /* the Bluetooth Stack that the second parameter is valid for (HCR   */
   /* Identifier), the Length of the Data to send and a pointer to the  */
   /* Data Buffer to Send.  This function returns the number of data    */
   /* bytes that were successfully sent, or a negative return error code*/
   /* if unsuccessful.                                                  */
   /* * NOTE * If there are not enough credits to send all of the       */
   /*          specified data then this function will return a number   */
   /*          less than the total number of bytes that were requested  */
   /*          to be sent.  This number could be zero.                  */
BTPSAPI_DECLARATION int BTPSAPI HCR_Data_Write(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t DataLength, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Data_Write_t)(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t DataLength, Byte_t *DataBuffer);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Server to respond to ANY received HCR Client        */
   /* Request with an PDU Status of Feature Unsupported.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack the*/
   /* HCR Server ID is valid with (specified by the second parameter).  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * This is a special function that is provided to allow a   */
   /*          mechanism to respond to any received PDU with an Feature */
   /*          Unsupported Reply PDU.  The Feature Unsupported PDU is a */
   /*          specially formatted PDU in that it only has the Reply    */
   /*          Status and no parameters.                                */
   /* * NOTE * This function can only be issued by an HCR Server.  HCR  */
   /*          Clients are NOT allowed to issue this function.          */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, the server *MUST*   */
   /*          respond to every Request that it receives.               */
   /* * NOTE * HCR Servers are NOT allowed to initiate ANY Control      */
   /*          Transactions.  HCR Servers can ONLY respond to Requests. */
BTPSAPI_DECLARATION int BTPSAPI HCR_Feature_Unsupported_Reply(unsigned int BluetoothStackID, unsigned int HCRID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Feature_Unsupported_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Client to send Vendor Specific Data to the remote   */
   /* HCR Server OR for the HCR Client or Server to send Vendor Specific*/
   /* Data to the remote HCR Client or Server on the Notification       */
   /* Channel (depending upon the value of the HCRID).  This function   */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack that the HCR ID is valid with (specified by the second      */
   /* parameter).  If the HCRID parameter specifies a HCR Client        */
   /* connection than the Vendor Specific Data is sent over the Control */
   /* Channel to the Remote HCR Server.  If the HCRID parameter         */
   /* specifies an HCR Notification Channel then the Data is sent over  */
   /* the Notification Channel (can be either side).  The third         */
   /* parameter specifies the PDU ID (must be greater than or equal to  */
   /* HCR_PDU_ID_VENDOR_SPECIFIC_MINIMUM and less than or equal to      */
   /* HCR_PDU_ID_VENDOR_SPECIFIC_MAXIMUM.  The fourth parameter         */
   /* specifies the number of Data bytes to send to the remote          */
   /* connection, and the final parameter specifies a pointer to a      */
   /* buffer that contains (at least) the number of valid data bytes to */
   /* send to the remote connection.  It is valid for the fourth        */
   /* parameter to specify zero, which signifies that there is no       */
   /* additional data to send within the PDU.  This function returns    */
   /* zero if the PDU was successfully sent, or a negative return value */
   /* if there was an error.                                            */
   /* * NOTE * If the HCRID parameter specifies an HCR Client ID then   */
   /*          this function can ONLY be issued by the HCR Client AND   */
   /*          NOT the HCR Server and the normal rules apply about only */
   /*          having a single request outstanding (i.e. another request*/
   /*          cannot be sent until a response from the Server is       */
   /*          received).                                               */
BTPSAPI_DECLARATION int BTPSAPI HCR_Vendor_Specific_Data_Request(unsigned int BluetoothStackID, unsigned int HCRID, Word_t PDUID, Word_t DataLength, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Vendor_Specific_Data_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t PDUID, Word_t DataLength, Byte_t *DataBuffer);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified HCR Entity to respond to a Vendor Specific Data Request */
   /* that was received.  This function accepts as input the Bluetooth  */
   /* Stack ID of the Bluetooth Stack the HCR Server ID is valid with   */
   /* (specified by the second parameter).  The third parameter to this */
   /* function specifies the status value to return to the Remote Entity*/
   /* (see constants above for defined values).  The fourth parameter   */
   /* specifies the number of Data bytes to send to the remote          */
   /* connection, and the final parameter specifies a pointer to a      */
   /* buffer that contains (at least) the number of valid data bytes to */
   /* send to the remote connection.  It is valid for the fourth        */
   /* parameter to specify zero, which signifies that there is no       */
   /* additional data to send within the PDU.  This function returns    */
   /* zero if the PDU was successfully sent, or a negative return value */
   /* if there was an error.                                            */
   /* * NOTE * Only one Action may be outstanding on the Control Channel*/
   /*          at any given time.  Because of this, once this function  */
   /*          has been called, no other Request can be originated until*/
   /*          a Reply is received from the Remote Server.              */
BTPSAPI_DECLARATION int BTPSAPI HCR_Vendor_Specific_Data_Reply(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, Word_t DataLength, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Vendor_Specific_Data_Reply_t)(unsigned int BluetoothStackID, unsigned int HCRID, Word_t ReplyStatusCode, Word_t DataLength, Byte_t *DataBuffer);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* sending the Notification Event on the Notification Channel.  This */
   /* function can only be sent by the entity that originated the       */
   /* Notification Connection (i.e.  the HCR Server on the Notification */
   /* Channel).  This function accepts as input the Bluetooth Stack ID  */
   /* of the Bluetooth Stack the HCR Notification is valid with         */
   /* (specified by the second parameter).  This function returns zero  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * After this Request is sent, the Connection will remain   */
   /*          active until the Server closes the Notification          */
   /*          Connection based upon the Notification Timeout.          */
   /* * NOTE * There is no response to this request.                    */
BTPSAPI_DECLARATION int BTPSAPI HCR_Notification_Request(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t CallbackContextID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Notification_Request_t)(unsigned int BluetoothStackID, unsigned int HCRID, DWord_t CallbackContextID);
#endif

   /* The following function is responsible for retrieving the current  */
   /* HCR Server Connection Mode.  This function accepts as its first   */
   /* parameter the Bluetooth Stack ID of the Bluetooth Stack on which  */
   /* the server exists.  The second parameter specifies the HCR Server */
   /* ID that the Server Mode is to be retrieved from.  The final       */
   /* parameter to this function is a pointer to a Server Connection    */
   /* Mode variable which will receive the current Server Connection    */
   /* Mode.  This function returns zero if successful, or a negative    */
   /* return error code if an error occurred.                           */
   /* ** NOTE ** The Default Server Connection Mode is                  */
   /*            csmAutomaticAccept.                                    */
   /* ** NOTE ** This function is used for HCR Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI HCR_Get_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int HCRID, HCR_Server_Connection_Mode_t *ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int HCRID, HCR_Server_Connection_Mode_t *ServerConnectionMode);
#endif

   /* The following function is responsible for setting the HCR Server  */
   /* Connection Mode.  This function accepts as its first parameter the*/
   /* Bluetooth Stack ID of the Bluetooth Stack in which the server     */
   /* exists.  The second parameter specifies the HCR Server ID of the  */
   /* HCR that is to have it's Server Mode changed.  The final parameter*/
   /* to this function is the new Server Connection Mode to set the     */
   /* Server to use.  Connection requests will not be dispatched unless */
   /* the Server Mode (second parameter) is set to csmManualAccept.  In */
   /* this case the Callback that was registered with the server will be*/
   /* invoked whenever a Remote Bluetooth Device attempts to connect to */
   /* the Local Device.  If the Server Mode (second parameter) is set to*/
   /* anything other than csmManualAccept then the then no connect      */
   /* request indication events will be dispatched.  This function      */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* ** NOTE ** The Default Server Connection Mode is                  */
   /*            csmAutomaticAccept.                                    */
   /* ** NOTE ** This function is used for HCR Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI HCR_Set_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int HCRID, HCR_Server_Connection_Mode_t ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HCR_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int HCRID, HCR_Server_Connection_Mode_t ServerConnectionMode);
#endif

#endif
