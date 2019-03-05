/*****< tdsapi.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSAPI  - Stonestreet One Bluetooth Stack 3D Synchronization (TDS) Type   */
/*            Definitions, Constants, and Prototypes.                         */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname       Description of Modification                  */
/*   --------  -----------       ---------------------------------------------*/
/*   04/01/13  R. Byrne          Initial creation.                            */
/*   04/15/14  T. Cook           Finalized API.                               */
/******************************************************************************/
#ifndef __TDSAPIH__
#define __TDSAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

#include "TDSTypes.h"           /* Bluetooth TDS Type Definitions/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTTDS_ERROR_INVALID_PARAMETER                             (-1000)
#define BTTDS_ERROR_NOT_INITIALIZED                               (-1001)
#define BTTDS_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTTDS_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTTDS_ERROR_DISPLAY_SERVER_ALREADY_OPEN                   (-1005)
#define BTTDS_ERROR_DISPLAY_SERVER_NOT_OPEN                       (-1006)
#define BTTDS_ERROR_NOT_ENABLED                                   (-1007)
#define BTTDS_ERROR_ISSUE_WITH_CONTROLLER                         (-1008)
#define BTTDS_INVALID_CSB_DATA                                    (-1009)
#define BTTDS_ERROR_LEGACY_MODE_ENABLED                           (-1010)

   /* SDP Profile UUID's for the 3D Synchronization Profile (3DSP).     */

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_16) to */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* TDS_PROTOCOL_UUID_16 Constant value.                              */
#define SDP_ASSIGN_TDS_PROFILE_UUID_16(_x)                     ASSIGN_SDP_UUID_16((_x), 0x11, 0x39)

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_32) to */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* TDS_PROTOCOL_UUID_32 Constant value.                              */
#define SDP_ASSIGN_TDS_PROFILE_UUID_32(_x)                     ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x39)

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_128) to*/
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* TDS_PROTOCOL_UUID_128 Constant value.                             */
#define SDP_ASSIGN_TDS_PROFILE_UUID_128(_x)                    ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x39, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP 3D Display Service Class UUID's.                              */

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_16) to */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* TDS_PROTOCOL_UUID_16 Constant value.                              */
#define SDP_ASSIGN_TDS_DISPLAY_UUID_16(_x)                     ASSIGN_SDP_UUID_16((_x), 0x11, 0x37)

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_32) to */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* TDS_PROTOCOL_UUID_32 Constant value.                              */
#define SDP_ASSIGN_TDS_DISPLAY_UUID_32(_x)                     ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x37)

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_128) to*/
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* TDS_PROTOCOL_UUID_128 Constant value.                             */
#define SDP_ASSIGN_TDS_DISPLAY_UUID_128(_x)                    ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x37, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP 3D Glasses Service Class UUID's.                              */

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_16) to */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* TDS_PROTOCOL_UUID_16 Constant value.                              */
#define SDP_ASSIGN_TDS_GLASSES_UUID_16(_x)                     ASSIGN_SDP_UUID_16((_x), 0x11, 0x38)

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_32) to */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* TDS_PROTOCOL_UUID_32 Constant value.                              */
#define SDP_ASSIGN_TDS_GLASSES_UUID_32(_x)                     ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x38)

   /* The following MACRO is a utility MACRO that assigns the 3DS       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_128) to*/
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* TDS_PROTOCOL_UUID_128 Constant value.                             */
#define SDP_ASSIGN_TDS_GLASSES_UUID_128(_x)                    ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x38, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* 3D Synchronization Profile Servers.                               */
#define TDS_PROFILE_VERSION                                    (0x0100)

   /* The following function represent the allowable bits that may be   */
   /* set in the flags passed to the TDS_Open_Display_Server() function.*/
   /* * NOTE * Only one of BRCM_OPTIMIZED and QCA_VS can be supplied    */
#define TDS_DISPLAY_SERVER_FLAGS_SUPPORT_BRCM_OPTIMIZED        0x00000001
#define TDS_DISPLAY_SERVER_FLAGS_SUPPORT_QCA_VS                0x00000002

   /* The following structure contains the Synchronization Train        */
   /* parameters that can be configured as part of this module.  The    */
   /* MinInterval, MaxInterval, and Timeout values should be specified  */
   /* in milliseconds.                                                  */
   /* * NOTE * See the function                                         */
   /*          TDS_Write_Synchronization_Train_Parameters for more      */
   /*          details.                                                 */
   /* * NOTE * Currently MinInterval and MaxInterval should be set to 80*/
   /*          ms for backwards compatibility with older 3D glasses.    */
   /* * NOTE * Timeout must be a value of at least 120 seconds          */
   /*          (specified in milliseconds).                             */
   /* * NOTE * All parameters are specified in milliseconds.            */
typedef struct _tagTDS_Synchronization_Train_Parameters_t
{
   Word_t  MinInterval;
   Word_t  MaxInterval;
   DWord_t Timeout;
   Byte_t  ServiceData;
} TDS_Synchronization_Train_Parameters_t;

   /* The following constant represents the minumum value of the Timeout*/
   /* field of the TDS_Synchronization_Train_Parameters_t.  This value  */
   /* equals 120 seconds.                                               */
#define TDS_SYNCHRONIZATION_TRAIN_TIMEOUT_MINIMUM                 (120000)

   /* The following structure contains the Connectionless Slave         */
   /* Broadcast parameters that can be configured as part of this       */
   /* module.  The MinInterval, MaxInterval, and SupervisionTimeout     */
   /* values should be specified in milliseconds.                       */
   /* * NOTE * See the function                                         */
   /*          TDS_Enable_Connectionless_Slave_Broadcast for more       */
   /*          details.                                                 */
   /* * NOTE * MinInterval should be a value of at least 50             */
   /*          milliseconds, and MaxInterval should be no greater than  */
   /*          100 milliseconds.                                        */
   /* * NOTE * The MinInterval, MaxInterval, and SupervisionTimeout     */
   /*          parameters should be specified in milliseconds.          */
typedef struct _tagTDS_Connectionless_Slave_Broadcast_Parameters_t
{
   Word_t MinInterval;
   Word_t MaxInterval;
   Word_t SupervisionTimeout;
   Word_t PacketType;
   Byte_t LowPower;
} TDS_Connectionless_Slave_Broadcast_Parameters_t;

   /* The following define the valid values of the LowPower field in the*/
   /* TDS_Connectionless_Slave_Broadcast_Parameters_t structure.        */
#define TDS_CSB_LOW_POWER_NOT_ALLOWED                             0x00
#define TDS_CSB_LOW_POWER_ALLOWED                                 0x01

   /* The following defines are the BR and EDR packets types that are   */
   /* used by the TDS_CSB_ENABLE_XXX() MACROs.                          */
#define TDS_CSB_PACKET_TYPE_BASIC_RATE                            (HCI_PACKET_ACL_TYPE_DM1 | HCI_PACKET_ACL_TYPE_DH1 | HCI_PACKET_ACL_TYPE_DM3 | HCI_PACKET_ACL_TYPE_DH3 | HCI_PACKET_ACL_TYPE_DM5 | HCI_PACKET_ACL_TYPE_DH5)
#define TDS_CSB_PACKET_TYPE_EDR_NOT_ALLOWED                       (HCI_PACKET_ACL_TYPE_2_DH1_MAY_NOT_BE_USED | HCI_PACKET_ACL_TYPE_3_DH1_MAY_NOT_BE_USED | HCI_PACKET_ACL_TYPE_2_DH3_MAY_NOT_BE_USED | HCI_PACKET_ACL_TYPE_3_DH3_MAY_NOT_BE_USED | HCI_PACKET_ACL_TYPE_2_DH5_MAY_NOT_BE_USED | HCI_PACKET_ACL_TYPE_3_DH5_MAY_NOT_BE_USED)

   /* The following MACROs exist to set the PacketType member of the    */
   /* TDS_Connectionless_Slave_Broadcast_Parameters_t structure.  The   */
   /* 4.1 specification that only Basic Rate (BR) packets shall be      */
   /* enabled or Enhanced Data Rate (EDR) and DM1 packet types shall be */
   /* enabled (i.e.  BR and EDR packet types should not be mixed).      */
   /* These two MACROs exist to enable either BR or EDR packet types.   */
#define TDS_CSB_ENABLE_BR_PACKETS(_x)                             ((_x) = (TDS_CSB_PACKET_TYPE_BASIC_RATE | TDS_CSB_PACKET_TYPE_EDR_NOT_ALLOWED))
#define TDS_CSB_ENABLE_EDR_PACKETS(_x)                            ((_x) = HCI_PACKET_ACL_TYPE_DM1)

   /* The following structure contains the fields required for          */
   /* configuring the Extended Inquiry Information returned by the      */
   /* Bluetooth controller.                                             */
   /* * NOTE * See the function TDS_Write_Extended_Inquiry_Information  */
   /*          for more details.                                        */
typedef struct _tagTDS_EIR_Information_t
{
   Byte_t  Flags;
   Byte_t  PathLossThreshold;
   SByte_t TxPowerLevel;
} TDS_EIR_Information_t;

   /* The following define the valid bitmask values of the Flags field  */
   /* in the TDS_EIR_Information_t structure.                           */
#define TDS_EIR_INFORMATION_FLAGS_ASSOCIATION_NOTIFICATION_SUPPORTED        0x01
#define TDS_EIR_INFORMATION_FLAGS_BATTERY_LEVEL_REPORTING_SUPPORTED         0x02
#define TDS_EIR_INFORMATION_FLAGS_SEND_3DG_CONNECTION_ANNOUNCEMENT_ON_SYNC  0x04
#define TDS_EIR_INFORMATION_FLAGS_IN_LEGACY_FACTORY_TEST_MODE               0x08
#define TDS_EIR_INFORMATION_FLAGS_IN_FACTORY_TEST_MODE                      0x80

   /* The following enumerated type represents the valid Video Modes.   */
typedef enum
{
   vm3D,
   vmDualView
} TDS_Video_Mode_t;

   /* The following structure contains the fields required for          */
   /* configuring the Connectionless Slave Broadcast Data that is used  */
   /* by the 3D Synchronization Profile.                                */
   /* * NOTE * The FrameSyncInstant should be the native Bluetooth clock*/
   /*          value (upper 27 bits).                                   */
   /* * NOTE * If the FrameSyncPeriod member is 0 then the              */
   /*          LeftLensShutterOpenOffset member must be set to          */
   /*          TDS_BROADCAST_MESSAGE_LEFT_LENS_OPEN_2D_MODE.            */
   /* * NOTE * When the Broadcom optimized mechanism for frame period   */
   /*          collection has been enabled on a Broadcom chip via the   */
   /*          TDS_DISPLAY_SERVER_FLAGS_SUPPORT_BRCM_OPTIMIZED being    */
   /*          passed to TDS_Open_Display_Server() then the             */
   /*          FrameSyncInstant, ClockPhase, FrameSyncPeriod and        */
   /*          FrameSyncPeriodFraction members are not used.            */
typedef struct _tagTDS_Connectionless_Slave_Broadcast_Data_t
{
   DWord_t          FrameSyncInstant;
   TDS_Video_Mode_t VideoMode;
   Word_t           ClockPhase;
   Word_t           LeftLensShutterOpenOffset;
   Word_t           LeftLensShutterCloseOffset;
   Word_t           RightLensShutterOpenOffset;
   Word_t           RightLensShutterCloseOffset;
   Word_t           FrameSyncPeriod;
   Byte_t           FrameSyncPeriodFraction;
} TDS_Connectionless_Slave_Broadcast_Data_t;

#define TDS_CONNECTIONLESS_SLAVE_BROADCAST_DATA_SIZE           (sizeof(TDS_Connectionless_Slave_Broadcast_Data_t))

   /* The following defines the maximum value of the ClockPhase member  */
   /* of the TDS_Connectionless_Slave_Broadcast_Data_t structure.       */
#define TDS_MAXIMUM_CLOCK_PHASE                                (624)

   /* The following defines the maxmimum value of the FrameSyncPeriod   */
   /* member of the TDS_Connectionless_Slave_Broadcast_Data_t structure.*/
#define TDS_MAXIMUM_FRAME_SYNC_PERIOD                          (40000)

   /* The following value may be used in the LeftLensShutterOpenOffset  */
   /* member TDS_Connectionless_Slave_Broadcast_Data_t of the to signal */
   /* that the glasses are in 2D mode and shall be opened.  When the    */
   /* Frame Sync field is 0 this value shall be used in the             */
   /* LeftLensShutterOpenOffset member according to the specification.  */
#define TDS_BROADCAST_MESSAGE_LEFT_LENS_OPEN_2D_MODE           (0xFFFF)

   /* TDS Event API Types.                                              */

   /* These events are issued to the application via the callback       */
   /* registered when the application opens a local or remote TDS device*/
   /* and/or a stream end point.                                        */
   /* * NOTE * The etTDS_Display_Slave_Page_Response_Timeout event      */
   /*          should be treated as a request from a 3DG to start the   */
   /*          synchronization train (for a duration of at least 120    */
   /*          seconds after this event).  This event can be ignored if */
   /*          the synchronization train is active when this event is   */
   /*          received.                                                */
   /* * NOTE * The etTDS_Display_Legacy_XXX events are only available   */
   /*          when the Broadcom optimized mode has been enabled on a   */
   /*          Broadcom chip via the                                    */
   /*          TDS_DISPLAY_SERVER_FLAGS_SUPPORT_BRCM_OPTIMIZED being    */
   /*          passed to TDS_Open_Display_Server().                     */
typedef enum
{
   etTDS_Display_Connection_Announcement,
   etTDS_Display_Synchronization_Train_Complete,
   etTDS_Display_CSB_Supervision_Timeout,
   etTDS_Display_Channel_Map_Change,
   etTDS_Display_Slave_Page_Response_Timeout,
   etTDS_Display_Triggered_Clock_Capture,
   etTDS_Display_Legacy_3D_Mode_Change,
   etTDS_Display_Legacy_Frame_Period,
   etTDS_Display_Legacy_Connection_Announcement
} TDS_Event_Type_t;

   /* TDS Events.                                                       */

   /* The following TDS Profile Event is dispatched when a Display      */
   /* receives a Connection Announcement over the registered L2CAP      */
   /* connectionless channel.  The Flags field contain a bitmask        */
   /* describing the connection announcement, and the BatteryLevel field*/
   /* indicate the percentage of power left in the connected glasses    */
   /* batteries.                                                        */
typedef struct _tagTDS_Display_Connection_Announcement_Data_t
{
   BD_ADDR_t BD_ADDR;
   Byte_t    Flags;
   Byte_t    BatteryLevel;
} TDS_Display_Connection_Announcement_Data_t;

#define TDS_DISPLAY_CONNECTION_ANNOUNCEMENT_DATA_SIZE              (sizeof(TDS_Display_Connection_Announcement_Data_t))

   /* The following define the valid bitmask values of the Flags field  */
   /* in the TDS_Server_Connection_Announcement_Data_t structure.       */
#define TDS_CONNECTION_ANNOUNCEMENT_FLAG_CONNECTION_FROM_ASSOCIATION       0x01
#define TDS_CONNECTION_ANNOUNCEMENT_FLAG_BATTERY_LEVEL_DISPLAY_REQUEST     0x02

   /* The following defines the special value in the BatteryLevel field */
   /* of the TDS_Display_Connection_Announcement_Data_t structure that  */
   /* indicates that Battery Level Reporting is not supporting.         */
#define TDS_CONNECTION_ANNOUNCEMENT_BATTERY_LEVEL_REPORTING_NOT_SUPPORTED  255

   /* The following TDS Profile Event is dispatched when a              */
   /* Synchronization Train request has completed from a call to the    */
   /* TDS_Start_Synchronization_Train function.  The Synchronization    */
   /* Train can be re-started if desired when this event is received.   */
typedef struct _tagTDS_Display_Synchronization_Train_Complete_Data_t
{
   Byte_t Status;
} TDS_Display_Synchronization_Train_Complete_Data_t;

#define TDS_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_DATA_SIZE      (sizeof(TDS_Display_Synchronization_Train_Complete_Data_t))

   /* The following TDS profile event is dispatched when a Channel Map  */
   /* Change occurs on the Connectionless Slave Broadcast Channel.  If  */
   /* this event is received, the application should restart the        */
   /* Synchronization Train in order to update the AFH Channel Map.     */
   /* * NOTE * Upon receiving this event the application may restart the*/
   /*          synchronization train to allow the receivers to received */
   /*          the updated AFH map.                                     */
typedef struct _tagTDS_Display_Channel_Map_Change_Data_t
{
   AFH_Channel_Map_t ChannelMap;
} TDS_Display_Channel_Map_Change_Data_t;

#define TDS_DISPLAY_CHANNEL_MAP_CHANGE_DATA_SIZE                  (sizeof(TDS_Display_Channel_Map_Change_Data_t))

   /* The following TDS profile event is dispatched when a Triggered    */
   /* Clock Capture event.  If this event is received, the application  */
   /* should adjust the 3D Connectionless Slave Broadcast data as       */
   /* necessary according to the timing.                                */
   /* * NOTE * The NativeBluetoothClock represents the native Bluetooth */
   /*          clock at the time of the triggering event.               */
   /* * NOTE * The SlotOffset represents the number of microseconds from*/
   /*          the NativeBluetoothClock clock attaining the value in    */
   /*          this event until the triggering event.                   */
typedef struct _tagTDS_Display_Triggered_Clock_Capture_Data_t
{
   DWord_t NativeBluetoothClock;
   Word_t  SlotOffset;
} TDS_Display_Triggered_Clock_Capture_Data_t;

#define TDS_DISPLAY_TRIGGERED_CLOCK_CAPTURE_DATA_SIZE             (sizeof(TDS_Display_Triggered_Clock_Capture_Data_t))

   /* The following structure represents the structure of the Legacy    */
   /* (Broadcom) 3D Fame Period Event Data.  This structure is used when*/
   /* a Legacy 3D Frame Period Vendor Specific Event is received.       */
typedef struct _tagTDS_Display_Legacy_Frame_Period_Event_Data_t
{
   Word_t Frame_Period;
   Byte_t Frame_Period_Fraction;
   Byte_t Unstable_Measurement;
} TDS_Display_Legacy_Frame_Period_Event_Data_t;

#define TDS_DISPLAY_LEGACY_FRAME_PERIOD_EVENT_DATA_SIZE              (sizeof(TDS_Display_Legacy_Frame_Period_Event_Data_t))

   /* The following structure represents the structure of the Legacy    */
   /* (Broadcom) Connection Announcement Event Data.  This structure is */
   /* used when a Legacy Connection Announcement Event is received.     */
typedef struct _tagTDS_Display_Legacy_Connection_Announcement_Event_Data_t
{
   BD_ADDR_t BD_ADDR;
   Byte_t    Device_Data;
   Byte_t    Reserved[10];
} TDS_Display_Legacy_Connection_Announcement_Event_Data_t;

#define TDS_DISPLAY_LEGACY_CONNECTION_ANNOUNCEMENT_EVENT_DATA_SIZE   (sizeof(TDS_Display_Legacy_Connection_Announcement_Event_Data_t))    

   /* The following structure represents the container structure for    */
   /* Holding all TDS Event Data Data.                                  */
typedef struct _tagTDS_Event_Data_t
{
   TDS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      TDS_Display_Connection_Announcement_Data_t                *TDS_Display_Connection_Announcement_Data;
      TDS_Display_Synchronization_Train_Complete_Data_t         *TDS_Display_Synchronization_Train_Complete_Data;
      TDS_Display_Channel_Map_Change_Data_t                     *TDS_Display_Channel_Map_Change_Data;
      TDS_Display_Triggered_Clock_Capture_Data_t                *TDS_Display_Triggered_Clock_Capture_Data;
      TDS_Display_Legacy_Frame_Period_Event_Data_t              *TDS_Display_Legacy_Frame_Period_Event_Data;
      TDS_Display_Legacy_Connection_Announcement_Event_Data_t   *TDS_Display_Legacy_Connection_Announcement_Event_Data;
   } Event_Data;
} TDS_Event_Data_t;

#define TDS_EVENT_DATA_SIZE                                       (sizeof(TDS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a TDS Event Receive Data Callback.  This function will be called  */
   /* whenever a TDS Event occurs that is associated with the specified */
   /* Bluetooth Stack ID.  This function passes to the caller the       */
   /* Bluetooth Stack ID, the TDS Event Data that occurred and the TDS  */
   /* Event Callback Parameter that was specified when this Callback was*/
   /* installed.  The caller is free to use the contents of the TDS     */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It needs to be noted,      */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another TDS Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving TDS Events.  A      */
   /*            deadlock WILL occur because NO TDS Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *TDS_Event_Callback_t)(unsigned int BluetoothStackID, TDS_Event_Data_t *TDS_Event_Data, unsigned long CallbackParameter);

   /* The following function is provided to allow a means to add a      */
   /* Display TDS Service Record to the SDP Database.  This function    */
   /* takes as input the Bluetooth Stack ID of the Local Bluetooth      */
   /* Protocol Stack as the first parameter.  The second parameter      */
   /* specifies the Service Name to associate with the SDP Record.  The */
   /* final parameter is a pointer to a DWord_t which receives the SDP  */
   /* Service Record Handle if this function successfully creates an SDP*/
   /* Service Record.  If this function returns zero, then the          */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          TDS_Un_Register_SDP_Record() to                          */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI TDS_Register_Display_SDP_Record(unsigned int BluetoothStackID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Register_Display_SDP_Record_t)(unsigned int BluetoothStackID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the TDS*/
   /* SDP Service Record (specified by the third parameter) from the SDP*/
   /* Database.  This MACRO simply maps to the                          */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* Service Record exists on and the SDP Service Record Handle.  The  */
   /* SDP Service Record Handle was returned via a succesful call to the*/
   /* TDS_Register_3DD_SDP_Record() function.  See the                  */
   /* TDS_Register_3DD_SDP_Record() functions for more information.     */
   /* This MACRO returns the result of the SDP_Delete_Service_Record()  */
   /* function, which is zero for success or a negative return error    */
   /* code (see BTERRORS.H).                                            */
#define TDS_Un_Register_SDP_Record(__BluetoothStackID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is provided to allow the local host to     */
   /* write the 3DS-specific extended inquiry information to the        */
   /* Bluetooth controller.  This is the data that the controller will  */
   /* return when it returns an extended inquiry response to a remote   */
   /* device.  This function takes as input the Bluetooth Stack ID of   */
   /* the Local Bluetooth Protocol Stack as the first parameter.  The   */
   /* second parameter specifies a pointer to a populated               */
   /* TDS_EIR_Information_t structure that contains the required values */
   /* for the EIR Data.  The third parameter specifes the length of any */
   /* additional data that will be appended to the packet buffer after  */
   /* the required fields are populated.  This value can be zero if no  */
   /* additional data is needed.  The forth parameter is a pointer to a */
   /* byte array that will be copied to the end of the EIR Data buffer. */
   /* This parameter can be NULL if AdditionalDataLength is zero.  This */
   /* function will return zero if successful, or a negative return     */
   /* error code if there was an error condition.                       */
   /* * NOTE * Any previous EIR data written to the Bluetooth controller*/
   /*          will be overwritten after this call succeeds.            */
BTPSAPI_DECLARATION int BTPSAPI TDS_Write_Extended_Inquiry_Information(unsigned int BluetoothStackID, TDS_EIR_Information_t *EIR_Information, unsigned int AdditionalDataLength, Byte_t *AdditionalData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Write_Extended_Inquiry_Information_t)(unsigned int BluetoothStackID, TDS_EIR_Information_t *EIR_Information, unsigned int AdditionalDataLength, Byte_t *AdditionalData);
#endif

   /* The following function will open a TDS Display server.  This      */
   /* function takes as input the Bluetooth Stack ID of the Local       */
   /* Bluetooth Protocol Stack as the first parameter.  The second      */
   /* parameters is a bitmask parameter that controls the configuration */
   /* of the display server.  The third parameter is the Callback       */
   /* function to call when an event occurs on this server.  The final  */
   /* parameters are a user-defined callback function and parameter that*/
   /* will be passed to the callback function with each event.  This    */
   /* function will return zero if successful, or a negative return     */
   /* error code if there was an error condition.                       */
   /* * NOTE * On success, this function will have reserved the LT_ADDR */
   /*          required for the Connectionless Slave Broadcast channel  */
   /*          and acquired the appropriate L2CAP fixed connectionless  */
   /*          channel.                                                 */
BTPSAPI_DECLARATION int BTPSAPI TDS_Open_Display_Server(unsigned int BluetoothStackID, unsigned long Flags, TDS_Event_Callback_t CallbackFunction, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Open_Display_Server_t)(unsigned int BluetoothStackID, unsigned long Flags, TDS_Event_Callback_t CallbackFunction, unsigned long CallbackParameter);
#endif

   /* The following function will close a previously opened TDS Display */
   /* server.  This function takes as input the Bluetooth Stack ID of   */
   /* the Local Bluetooth Protocol Stack as the first parameter.  This  */
   /* function will return zero if successful, or a negative return     */
   /* error code if there was an error condition.                       */
   /* * NOTE * On success, this function will have deleted the          */
   /*          reservation for the LT_ADDR required for the             */
   /*          Connectionless Slave Broadcast channel and released the  */
   /*          appropriate L2CAP fixed connectionless channel.          */
BTPSAPI_DECLARATION int BTPSAPI TDS_Close_Display_Server(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Close_Display_Server_t)(unsigned int BluetoothStackID);
#endif

   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  This function takes as   */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol Stack*/
   /* as the first parameter.  The second parameter is a pointer to the */
   /* Synchronization Train parameters to be set.  The third parameter  */
   /* is a pointer to the variable that will be populated with the      */
   /* Synchronization Interval chosen by the Bluetooth controller.  This*/
   /* function will return zero on success; otherwise, a negative error */
   /* value will be returned.                                           */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
BTPSAPI_DECLARATION int BTPSAPI TDS_Write_Synchronization_Train_Parameters(unsigned int BluetoothStackID, TDS_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Write_Synchronization_Train_Parameters_t)(unsigned int BluetoothStackID, TDS_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult);
#endif

   /* The following function will enable the Synchronization Train.     */
   /* This function takes as input the Bluetooth Stack ID of the local  */
   /* Bluetooth Protocol Stack as the first parameter.  This function   */
   /* will return zero on success; otherwise, a negative error value    */
   /* will be returned.                                                 */
   /* * NOTE * The TDS_Write_Synchronization_Train_Parameters function  */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The etTDS_Display_Synchronization_Train_Complete event   */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
BTPSAPI_DECLARATION int BTPSAPI TDS_Start_Synchronization_Train(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Start_Synchronization_Train_t)(unsigned int BluetoothStackID);
#endif

   /* The following function will set the Connectionless Slave Broadcast*/
   /* Data that will be transmitted every interval specified by the     */
   /* return parameter from the function call                           */
   /* TDS_Write_Synchronization_Train_Parameters.  The same data will be*/
   /* transmitted until new data is written.  This function takes as    */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol Stack*/
   /* as the first parameter.  The second parameter is a pointer to the */
   /* Broadcast Message Info structure that contains the fields that    */
   /* will populate the Broadcast Message Packet.  This function will   */
   /* return zero on success; otherwise, a negative value is returned.  */
BTPSAPI_DECLARATION int BTPSAPI TDS_Set_Connectionless_Slave_Broadcast_Data(unsigned int BluetoothStackID, TDS_Connectionless_Slave_Broadcast_Data_t *ConnectionSlaveData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Set_Connectionless_Slave_Broadcast_Data_t)(unsigned int BluetoothStackID, TDS_Connectionless_Slave_Broadcast_Data_t *ConnectionSlaveData);
#endif

   /* The following function will configure and enable the              */
   /* Connectionless Slave Broadcast channel on the Bluetooth           */
   /* controller.  This function takes as input the Bluetooth Stack ID  */
   /* of the Local Bluetooth Protocol Stack as the first parameter.  The*/
   /* second parameter is a pointer to the Connectionless Slave         */
   /* Broadcast parameters to be set.  The third parameter is a pointer */
   /* to the variable that will be populated with the Broadcast Interval*/
   /* chosen by the Bluetooth controller.  This function will return    */
   /* zero on success; otherwise, a negative error value will be        */
   /* returned.                                                         */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
BTPSAPI_DECLARATION int BTPSAPI TDS_Enable_Connectionless_Slave_Broadcast(unsigned int BluetoothStackID, TDS_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Enable_Connectionless_Slave_Broadcast_t)(unsigned int BluetoothStackID, TDS_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult);
#endif

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.  This function takes as   */
   /* input the Bluetooth Stack ID of the local Bluetooth Protocol Stack*/
   /* as the first parameter.                                           */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
BTPSAPI_DECLARATION int BTPSAPI TDS_Disable_Connectionless_Slave_Broadcast(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Disable_Connectionless_Slave_Broadcast_t)(unsigned int BluetoothStackID);
#endif

   /* The following function will configure and enable the triggered    */
   /* clock capture feature to convert an external input to the         */
   /* controller into the native Bluetooth clock.  This function takes  */
   /* as input the Bluetooth Stack ID of the Local Bluetooth Protocol   */
   /* Stack as the first parameter.  The second parameter is the number */
   /* of Clock Capture filter count.  This function will return zero on */
   /* success; otherwise, a negative error value will be returned.      */
   /* * NOTE * The etTDS_Display_Triggered_Clock_Capture will be        */
   /*          generated with the native Bluetooth clock periodically   */
   /*          once this function is called until the                   */
   /*          TDS_Disable_Triggered_Clock_Capture() API is called.     */
   /* * NOTE * The ClockCaptureFilterCount is used to filter triggered  */
   /*          clock capture events.  After sending a triggered clock   */
   /*          event to the Host the host will filter the next          */
   /*          ClockCaptureFilterCount event before sending the next    */
   /*          triggered clock capture event.  A value of 0 indicates   */
   /*          that every triggered clock capture should be dispatched  */
   /*          to the application.                                      */
BTPSAPI_DECLARATION int BTPSAPI TDS_Start_Triggered_Clock_Capture(unsigned int BluetoothStackID, Byte_t ClockCaptureFilterCount);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Start_Triggered_Clock_Capture_t)(unsigned int BluetoothStackID, Byte_t ClockCaptureFilterCount);
#endif

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.  This function takes as   */
   /* input the Bluetooth Stack ID of the local Bluetooth Protocol Stack*/
   /* as the first parameter.                                           */
   /* * NOTE * Calling this function will terminate the triggered clock */
   /*          capture feature in the controller (if it is currently    */
   /*          enabled).                                                */
BTPSAPI_DECLARATION int BTPSAPI TDS_Disable_Triggered_Clock_Capture(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDS_Disable_Triggered_Clock_Capture_t)(unsigned int BluetoothStackID);
#endif

#endif
