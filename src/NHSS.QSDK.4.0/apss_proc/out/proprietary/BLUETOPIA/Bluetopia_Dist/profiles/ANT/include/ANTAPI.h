/*****< antapi.h >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTAPI - Stonestreet One Bluetooth Stack ANT+ extension Type Definitions, */
/*           Prototypes, and Constants.                                       */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/17/11  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANTAPIH__
#define __ANTAPIH__

#include "SS1BTPS.h"           /* Bluetooth Stack API Prototypes/Constants.   */

#include "ANTTypes.h"          /* ANT+ Type Definitions and Constants.        */

   /* The following define the error codes that may be returned by this */
   /* module.                                                           */
#define ANT_ERROR_SUCCESS                                       (0)

#define BTANT_ERROR_INVALID_PARAMETER                       (-1000)
#define BTANT_ERROR_INVALID_BLUETOOTH_STACK_ID              (-1001)
#define BTANT_ERROR_NOT_INITIALIZED                         (-1002)
#define BTANT_ERROR_INSUFFICIENT_RESOURCES                  (-1003)
#define BTANT_ERROR_UNABLE_TO_SEND_ANT_MESSAGE              (-1004)
#define BTANT_ERROR_ANT_MESSAGE_RESPONSE_ERROR              (-1005)
#define BTANT_ERROR_INVALID_NETWORK_KEY                     (-1006)
#define BTANT_ERROR_FEATURE_NOT_ENABLED                     (-1007)

   /* The following constants represent the define flag bit-mask values */
   /* that can be passed to the ANT_Initialize() function.              */
#define ANT_INITIALIZATION_FLAGS_RAW_MODE                   0x00000001L

   /* ANT Event API Types.                                              */
typedef enum
{
   etStartup_Message,
   etChannel_Response,
   etChannel_Status,
   etChannel_ID,
   etANT_Version,
   etCapabilities,
   etPacket_Broadcast_Data,
   etPacket_Acknowledged_Data,
   etPacket_Burst_Data,
   etPacket_Extended_Broadcast_Data,
   etPacket_Extended_Acknowledged_Data,
   etPacket_Extended_Burst_Data,
   etRaw_Packet_Data
} ANT_Event_Type_t;

   /* ANT Event Data definitions.                                       */

typedef struct _tagANT_Startup_Message_Event_Data_t
{
   Byte_t Startup_Message;
} ANT_Startup_Message_Event_Data_t;

#define ANT_STARTUP_MESSAGE_EVENT_DATA_SIZE                 (sizeof(ANT_Startup_Message_Event_Data_t))

typedef struct _tagANT_Channel_Response_Event_Data_t
{
   Byte_t Channel_Number;
   Byte_t Message_ID;
   Byte_t Message_Code;
} ANT_Channel_Response_Event_Data_t;

#define ANT_CHANNEL_RESPONSE_EVENT_DATA_SIZE                (sizeof(ANT_Channel_Response_Event_Data_t))

typedef struct _tagANT_Channel_Status_Event_Data_t
{
   Byte_t Channel_Number;
   Byte_t Channel_Status;
} ANT_Channel_Status_Event_Data_t;

#define ANT_CHANNEL_STATUS_EVENT_DATA_SIZE                  (sizeof(ANT_Channel_Status_Event_Data_t))

typedef struct _tagANT_Channel_ID_Event_Data_t
{
   Byte_t Channel_Number;
   Word_t Device_Number;
   Byte_t Device_Type_ID;
   Byte_t Transmission_Type;
} ANT_Channel_ID_Event_Data_t;

#define ANT_CHANNEL_ID_EVENT_DATA_SIZE                      (sizeof(ANT_Channel_ID_Event_Data_t))

typedef struct _tagANT_Version_Event_Data_t
{
   char Version_Data[11];
} ANT_Version_Event_Data_t;

#define ANT_VERSION_EVENT_DATA_SIZE                         (sizeof(ANT_Version_Event_Data_t))

typedef struct _tagANT_Capabilities_Event_Data_t
{
   Byte_t Max_Channels;
   Byte_t Max_Networks;
   Byte_t Standard_Options;
   Byte_t Advanced_Options;
   Byte_t Advanced_Options2;
   Byte_t Reserved;
} ANT_Capabilities_Event_Data_t;

#define ANT_CAPABILITIES_EVENT_DATA_SIZE                    (sizeof(ANT_Capabilities_Event_Data_t))

typedef struct _tagANT_Packet_Broadcast_Data_Event_Data_t
{
   Byte_t Channel_Number;
   Byte_t Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
} ANT_Packet_Broadcast_Data_Event_Data_t;

#define ANT_PACKET_BROADCAST_DATA_EVENT_DATA_SIZE           (sizeof(ANT_Packet_Broadcast_Data_Event_Data_t))

typedef struct _tagANT_Packet_Acknowledged_Data_Event_Data_t
{
   Byte_t Channel_Number;
   Byte_t Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
} ANT_Packet_Acknowledged_Data_Event_Data_t;

#define ANT_PACKET_ACKNOWLEDGED_DATA_EVENT_DATA_SIZE        (sizeof(ANT_Packet_Acknowledged_Data_Event_Data_t))

typedef struct _tagANT_Packet_Burst_Data_Event_Data_t
{
   Byte_t Sequence_Channel_Number;
   Byte_t Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
} ANT_Packet_Burst_Data_Event_Data_t;

#define ANT_PACKET_BURST_DATA_EVENT_DATA_SIZE               (sizeof(ANT_Packet_Burst_Data_Event_Data_t))

typedef struct _tagANT_Packet_Extended_Broadcast_Data_Event_Data_t
{
   Byte_t Channel_Number;
   Word_t Device_Number;
   Byte_t Device_Type;
   Byte_t Transmission_Type;
   Byte_t Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
} ANT_Packet_Extended_Broadcast_Data_Event_Data_t;

#define ANT_PACKET_EXTENDED_BROADCAST_DATA_EVENT_DATA_SIZE  (sizeof(ANT_Packet_Extended_Broadcast_Data_Event_Data_t))

typedef struct _tagANT_Packet_Extended_Acknowledged_Data_Event_Data_t
{
   Byte_t Channel_Number;
   Word_t Device_Number;
   Byte_t Device_Type;
   Byte_t Transmission_Type;
   Byte_t Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
} ANT_Packet_Extended_Acknowledged_Data_Event_Data_t;

#define ANT_PACKET_EXTENDED_ACKNOWLEDGED_DATA_EVENT_DATA_SIZE  (sizeof(ANT_Packet_Extended_Acknowledged_Data_Event_Data_t))

typedef struct _tagANT_Packet_Extended_Burst_Data_Event_Data_t
{
   Byte_t Sequence_Channel_Number;
   Word_t Device_Number;
   Byte_t Device_Type;
   Byte_t Transmission_Type;
   Byte_t Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
} ANT_Packet_Extended_Burst_Data_Event_Data_t;

#define ANT_PACKET_EXTENDED_BURST_DATA_EVENT_DATA_SIZE      (sizeof(ANT_Packet_Extended_Burst_Data_Event_Data_t))

typedef struct _tagANT_Raw_Packet_Data_Event_Data_t
{
   unsigned int  PacketLength;
   Byte_t       *PacketData;
} ANT_Raw_Packet_Data_Event_Data_t;

#define ANT_RAW_PACKET_DATA_EVENT_DATA_SIZE                 (sizeof(ANT_Raw_Packet_Data_Event_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all ANT Event Data Data.                                  */
typedef struct _tagANT_Event_Data_t
{
   ANT_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      ANT_Startup_Message_Event_Data_t                   *ANT_Startup_Message_Event_Data;
      ANT_Channel_Response_Event_Data_t                  *ANT_Channel_Response_Event_Data;
      ANT_Channel_Status_Event_Data_t                    *ANT_Channel_Status_Event_Data;
      ANT_Channel_ID_Event_Data_t                        *ANT_Channel_ID_Event_Data;
      ANT_Version_Event_Data_t                           *ANT_Version_Event_Data;
      ANT_Capabilities_Event_Data_t                      *ANT_Capabilities_Event_Data;
      ANT_Packet_Broadcast_Data_Event_Data_t             *ANT_Packet_Broadcast_Data_Event_Data;
      ANT_Packet_Acknowledged_Data_Event_Data_t          *ANT_Packet_Acknowledged_Data_Event_Data;
      ANT_Packet_Burst_Data_Event_Data_t                 *ANT_Packet_Burst_Data_Event_Data;
      ANT_Packet_Extended_Broadcast_Data_Event_Data_t    *ANT_Packet_Extended_Broadcast_Data_Event_Data;
      ANT_Packet_Extended_Acknowledged_Data_Event_Data_t *ANT_Packet_Extended_Acknowledged_Data_Event_Data;
      ANT_Packet_Extended_Burst_Data_Event_Data_t        *ANT_Packet_Extended_Burst_Data_Event_Data;
      ANT_Raw_Packet_Data_Event_Data_t                   *ANT_Raw_Packet_Data_Event_Data;
   } Event_Data;
} ANT_Event_Data_t;

#define ANT_EVENT_DATA_SIZE                                 (sizeof(ANT_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an ANT Event Receive Data Callback.  This function will be called */
   /* whenever an ANT Event occurs that is associated with the specified*/
   /* Bluetooth Stack ID.  This function passes to the caller the       */
   /* Bluetooth Stack ID, the ANT Event Data that occurred and the ANT  */
   /* Event Callback Parameter that was specified when this Callback was*/
   /* installed.  The caller is free to use the contents of the ANT     */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another ANT Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving ANT Event Packets.  */
   /*            A Deadlock WILL occur because NO ANT Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
typedef void (BTPSAPI *ANT_Event_Callback_t)(unsigned int BluetoothStackID, ANT_Event_Data_t *ANT_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for initializing an ANT     */
   /* Context Layer for the specified Bluetooth Protocol Stack.  This   */
   /* function will allocate and initialize a ANT Context Information   */
   /* structure and initialize the structure members.  The function     */
   /* accepts a bit-mask of initialization flags and an ANT Event       */
   /* Callback function (and callback parameter).  This callback is     */
   /* called whenever an ANT Event occurs.  This function returns zero  */
   /* if successful, or a non-zero value if there was an error.         */
   /* * NOTE * This function can be called (and the module initialized) */
   /*          without the ANT Feature being enabled.                   */
   /* * NOTE * The Flags parameter must be made up of bitmasks of the   */
   /*          ANT_INITIALIZATION_FLAGS_XXX or 0.                       */
BTPSAPI_DECLARATION int BTPSAPI ANT_Initialize(unsigned int BluetoothStackID, unsigned long Flags, ANT_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Initialize_t)(unsigned int BluetoothStackID, unsigned long Flags, ANT_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for releasing any resources */
   /* that the ANT Layer, associated with the Bluetooth Protocol Stack, */
   /* specified by the Bluetooth Stack ID, has allocated.  Upon         */
   /* completion of this function, ALL ANT functions will fail if used  */
   /* on the specified Bluetooth Protocol Stack.                        */
   /* * NOTE * This function can NOT be called when the ANT Feature is  */
   /*          enabled.  If it is called in this context, then this     */
   /*          function will fail to clean up the module.               */
BTPSAPI_DECLARATION void BTPSAPI ANT_Cleanup(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANT_Cleanup_t)(unsigned int BluetoothStackID);
#endif

   /* Configuration Message API.                                        */

   /* The following function is responsible for assigning an ANT channel*/
   /* on the Bluetooth device specified by BluetoothStackID.  This      */
   /* function accepts as it's second argument, the channel number to   */
   /* register.  This function accepts as it's third argument, the      */
   /* channel type to be assigned to the channel.  This function accepts*/
   /* as it's fourth argument, the network number to be used for the    */
   /* channel.  Zero should be specified for this argument to use the   */
   /* default public network.  This function accepts as it's fifth      */
   /* argument, the extended assignment to be used for the channel.     */
   /* Zero should be specified for this argument if no extended         */
   /* capabilities are to be used.  This function returns zero if the   */
   /* Packet was successfully sent, otherwise this function returns a   */
   /* negative error code.                                              */
BTPSAPI_DECLARATION int BTPSAPI ANT_Assign_Channel(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t ChannelType, Byte_t NetworkNumber, Byte_t ExtendedAssignment);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Assign_Channel_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t ChannelType, Byte_t NetworkNumber, Byte_t ExtendedAssignment);
#endif

   /* The following function is responsible for unassigning an ANT      */
   /* channel on the Bluetooth device specified by BluetoothStackID.  A */
   /* channel must be unassigned before it can be reassigned using      */
   /* ANT_Assign_Channel.  This function accepts as it's second         */
   /* argument, the channel number to unregister.  This function returns*/
   /* zero if the Packet was successfully sent, otherwise this function */
   /* returns a negative error code.                                    */
BTPSAPI_DECLARATION int BTPSAPI ANT_Unassign_Channel(unsigned int BluetoothStackID, Byte_t ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Unassign_Channel_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber);
#endif

   /* The following function is responsible for configuring an ANT      */
   /* channel on the Bluetooth device specified by BluetoothStackID.    */
   /* This function accepts as it's second argument, the channel        */
   /* number to configure.  The ANT channel must be assigned using      */
   /* ANT_Assign_Channel before calling this function.  This function   */
   /* accepts as it's third argument, the device number to search for   */
   /* on the channel.  Zero should be specified for this argument to    */
   /* scan for any device number.  This function accepts as it's fourth */
   /* argument, the device type to search for on the channel.  Zero     */
   /* should be specified for this argument to scan for any device type.*/
   /* This function accepts as it's fifth argument, the transmission    */
   /* type to search for on the channel.  Zero should be specified for  */
   /* this argument to scan for any transmission type.  This function   */
   /* returns zero if the Packet was successfully sent, otherwise this  */
   /* function returns a negative error code.                           */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Channel_ID(unsigned int BluetoothStackID, Byte_t ChannelNumber, Word_t DeviceNumber, Byte_t DeviceType, Byte_t TransmissionType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Channel_ID_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Word_t DeviceNumber, Byte_t DeviceType, Byte_t TransmissionType);
#endif

   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the Bluetooth device       */
   /* specified by BluetoothStackID.  This function accepts as it's     */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel messaging period to   */
   /* set on the channel.  This function returns zero if the Packet was */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Channel_Period(unsigned int BluetoothStackID, Byte_t ChannelNumber, Word_t MessagingPeriod);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Channel_Period_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Word_t MessagingPeriod);
#endif

   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument, the    */
   /* Bluetooth device specified by BluetoothStackID.  This function    */
   /* accepts as it's second argument, the channel number to configure. */
   /* This function accepts as it's third argument, the search timeout  */
   /* to set on the channel.  This function returns zero if the Packet  */
   /* was successfully sent, otherwise this function returns a negative */
   /* error code.                                                       */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Channel_Search_Timeout(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Channel_Search_Timeout_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchTimeout);
#endif

   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel on the Bluetooth device specified by */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to configure.  This function accepts as it's   */
   /* third argument, the channel frequency to set on the channel.  This*/
   /* function returns zero if the Packet was successfully sent,        */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Channel_RF_Frequency(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t RFFrequency);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Channel_RF_Frequency_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t RFFrequency);
#endif

   /* The following function is responsible for configuring the network */
   /* key for an ANT channel on the Bluetooth device specified by       */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to configure.  This function accepts as it's   */
   /* third argument, a pointer to the ANT network key to set on the    */
   /* channel.  This function returns zero if the Packet was            */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Network_Key(unsigned int BluetoothStackID, Byte_t NetworkNumber, ANT_Network_Key_t NetworkKey);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Network_Key_t)(unsigned int BluetoothStackID, Byte_t NetworkNumber, ANT_Network_Key_t NetworkKey);
#endif

   /* The following function is responsible for configuring the transmit*/
   /* power on the Bluetooth device specified by BluetoothStackID.  This*/
   /* function accepts as it's second argument, the transmit power to   */
   /* set on the device.  This function returns zero if the Packet was  */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Transmit_Power(unsigned int BluetoothStackID, Byte_t TransmitPower);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Transmit_Power_t)(unsigned int BluetoothStackID, Byte_t TransmitPower);
#endif

   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function        */
   /* accepts as it's first argument, the Bluetooth device specified by */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to add to the list.  This function accepts as  */
   /* it's third argument, the device number to add to the list.  This  */
   /* function accepts as it's fourth argument, the device type to add  */
   /* to the list.  This function accepts as it's fifth argument, the   */
   /* transmission type to add to the list.  This function accepts as   */
   /* it's sixth argument, the the list index to overwrite with the     */
   /* updated entry.  This function returns zero if the Packet was      */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANT_Add_Channel_ID(unsigned int BluetoothStackID, Byte_t ChannelNumber, Word_t DeviceNumber, Byte_t DeviceType, Byte_t TransmissionType, Byte_t ListIndex);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Add_Channel_ID_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Word_t DeviceNumber, Byte_t DeviceType, Byte_t TransmissionType, Byte_t ListIndex);
#endif

   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the Bluetooth device specified by   */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number on which the list should be configured.  This  */
   /* function accepts as it's third argument, the size of the list.    */
   /* This function accepts as it's fourth argument, the list type.     */
   /* Zero should be specified to configure the list for inclusion, and */
   /* one should be specified to configure the list for exclusion.  This*/
   /* function returns zero if the Packet was successfully sent,        */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANT_Config_List(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t ListSize, Byte_t Exclude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Config_List_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t ListSize, Byte_t Exclude);
#endif

   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel on the Bluetooth device specified by     */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to configure.  This function accepts as it's   */
   /* third argument, the transmit power level for the specified        */
   /* channel.  This function returns zero if the Packet was            */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Channel_Transmit_Power(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t TransmitPower);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Channel_Transmit_Power_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t TransmitPower);
#endif

   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument, the Bluetooth device specified by */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to configure.  This function accepts as it's   */
   /* third argument, the search timeout to set on the channel.  This   */
   /* function returns zero if the Packet was successfully sent,        */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Low_Priority_Channel_Search_Timeout(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Low_Priority_Channel_Search_Timeout_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchTimeout);
#endif

   /* The following function is responsible for configuring an ANT      */
   /* channel on the Bluetooth device specified by BluetoothStackID.    */
   /* This function configures the channel ID in the same way as        */
   /* ANT_Set_Channel_ID, except it uses the two LSB of the device's    */
   /* serial number as the device's number.  This function accepts as   */
   /* it's second argument, the channel number to configure.  The ANT   */
   /* channel must be assigned using ANT_Assign_Channel before calling  */
   /* this function.  This function accepts as it's third argument, the */
   /* device type to search for on the channel.  Zero should be         */
   /* specified for this argument to scan for any device type.  This    */
   /* function accepts as it's fourth argument, the transmission type to*/
   /* search for on the channel.  Zero should be specified for this     */
   /* argument to scan for any transmission type.  This function returns*/
   /* zero if the Packet was successfully sent, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Serial_Number_Channel_ID(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t DeviceType, Byte_t TransmissionType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Serial_Number_Channel_ID_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t DeviceType, Byte_t TransmissionType);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the Bluetooth device   */
   /* specified by BluetoothStackID.  This function accepts as it's     */
   /* second argument, whether or not to enable extended Rx messages.   */
   /* This function returns zero if the Packet was successfully sent,   */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANT_Enable_Extended_Messages(unsigned int BluetoothStackID, Byte_t Enable);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Enable_Extended_Messages_t)(unsigned int BluetoothStackID, Byte_t Enable);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* the LED on the Bluetooth device specified by BluetoothStackID.    */
   /* This function accepts as it's second argument, whether or not to  */
   /* enable the LED.  This function returns zero if the Packet was     */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANT_Enable_LED(unsigned int BluetoothStackID, Byte_t Enable);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Enable_LED_t)(unsigned int BluetoothStackID, Byte_t Enable);
#endif

   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the Bluetooth device specified by                */
   /* BluetoothStackID.  This function returns zero if the Packet was   */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
BTPSAPI_DECLARATION int BTPSAPI ANT_Crystal_Enable(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Crystal_Enable_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the Bluetooth device specified by     */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the bitmask of extended Rx messages that shall be enabled or      */
   /* disabled.  This function returns zero if the Packet was           */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANT_Lib_Config(unsigned int BluetoothStackID, Byte_t LibConfig);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Lib_Config_t)(unsigned int BluetoothStackID, Byte_t LibConfig);
#endif

   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel on the Bluetooth device  */
   /* specified by BluetoothStackID.  This function accepts as it's     */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third, forth, and fifth arguments, the three      */
   /* operating agility frequencies to set.  This function returns zero */
   /* if the Packet was successfully sent, otherwise this function      */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANT_Assign_Channel.  Frequency agility should*/
   /*          NOT be used with shared, Tx only, or Rx only channels.   */
BTPSAPI_DECLARATION int BTPSAPI ANT_Config_Frequency_Agility(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t FrequencyAgility1, Byte_t FrequencyAgility2, Byte_t FrequencyAgility3);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Config_Frequency_Agility_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t FrequencyAgility1, Byte_t FrequencyAgility2, Byte_t FrequencyAgility3);
#endif

   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the Bluetooth device specified by */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to configure.  This function accepts as it's   */
   /* third argument, the search threshold to set.  This function       */
   /* returns zero if the Packet was successfully sent, otherwise this  */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Proximity_Search(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchThreshold);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Proximity_Search_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchThreshold);
#endif

   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the Bluetooth device specified by   */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to configure.  This function accepts as it's   */
   /* third argument, the search priority to set.  This function returns*/
   /* zero if the Packet was successfully sent, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_Channel_Search_Priority(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchPriority);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_Channel_Search_Priority_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t SearchPriority);
#endif

   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the Bluetooth device specified by            */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the descriptor string type to set.  This function accepts as it's */
   /* third argument, the NULL-terminated descriptor string to be set.  */
   /* This function returns zero if the Packet was successfully sent,   */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_USB_Descriptor_String(unsigned int BluetoothStackID, Byte_t StringNumber, char *DescriptorString);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_USB_Descriptor_String_t)(unsigned int BluetoothStackID, Byte_t StringNumber, char *DescriptorString);
#endif

   /* Control Message API.                                              */

   /* The following function is responsible for resetting the ANT module*/
   /* on the Bluetooth device specified by BluetoothStackID.  A delay of*/
   /* at least 500ms is suggested after calling this function to allow  */
   /* time for the module to reset.  This function returns zero if the  */
   /* Packet was successfully sent, otherwise this function returns a   */
   /* negative error code.                                              */
BTPSAPI_DECLARATION int BTPSAPI ANT_Reset_System(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Reset_System_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for opening an ANT channel  */
   /* on the Bluetooth device specified by BluetoothStackID.  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  The channel specified must have been assigned and        */
   /* configured before calling this function.  This function returns   */
   /* zero if the Packet was successfully sent, otherwise this function */
   /* returns a negative error code.                                    */
BTPSAPI_DECLARATION int BTPSAPI ANT_Open_Channel(unsigned int BluetoothStackID, Byte_t ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Open_Channel_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber);
#endif

   /* The following function is responsible for closing an ANT channel  */
   /* on the Bluetooth device specified by BluetoothStackID.  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  This function returns zero if the Packet was successfully*/
   /* sent, otherwise this function returns a negative error code.      */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the etChannel_Response event has been received with*/
   /*          the Message_Code member specifying:                      */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
BTPSAPI_DECLARATION int BTPSAPI ANT_Close_Channel(unsigned int BluetoothStackID, Byte_t ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Close_Channel_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber);
#endif

   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the Bluetooth device   */
   /* specified by BluetoothStackID.  This function accepts as it's     */
   /* second argument, the channel number that the request will be sent */
   /* to.  This function accepts as it's third argument, the message ID */
   /* being requested from the channel.  This function returns zero if  */
   /* the Packet was successfully sent, otherwise this function returns */
   /* a negative error code.                                            */
BTPSAPI_DECLARATION int BTPSAPI ANT_Request_Message(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t MessageID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Request_Message_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t MessageID);
#endif

   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the Bluetooth device specified by      */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number to be opened.  The channel specified must have */
   /* been assigned and configured as a SLAVE Rx ONLY channel before    */
   /* calling this function.  This function returns zero if the Packet  */
   /* was successfully sent, otherwise this function returns a negative */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
BTPSAPI_DECLARATION int BTPSAPI ANT_Open_Rx_Scan_Mode(unsigned int BluetoothStackID, Byte_t ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Open_Rx_Scan_Mode_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber);
#endif

   /* The following function is responsible for putting the ANT module  */
   /* in ultra low-power mode.  This function accepts as it's first     */
   /* argument, the Bluetooth device specified by BluetoothStackID.     */
   /* This function returns zero if the Packet was successfully sent,   */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
BTPSAPI_DECLARATION int BTPSAPI ANT_Sleep_Message(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Sleep_Message_t)(unsigned int BluetoothStackID);
#endif

   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the Bluetooth device specified by          */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number that the data will be broadcast on.  This      */
   /* function accepts as it's third argument, a pointer to a byte array*/
   /* of the broadcast data to send.  The data array MUST be at least   */
   /* eight bytes in length, and only the first eight bytes will be     */
   /* broadcast.  This function returns zero if the Packet was          */
   /* successfully sent, otherwise this function returns a negative     */
   /* error code.                                                       */
BTPSAPI_DECLARATION int BTPSAPI ANT_Send_Broadcast(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Send_Broadcast_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t *Data);
#endif

   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the Bluetooth device specified by     */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the channel number that the data will be sent on.  This function  */
   /* accepts as it's third argument, a pointer to a byte array of the  */
   /* acknowledged data to send.  The data array MUST be at least eight */
   /* bytes in length, and only the first eight bytes will be sent.     */
   /* This function returns zero if the Packet was successfully sent,   */
   /* otherwise this function returns a negative error code.            */
BTPSAPI_DECLARATION int BTPSAPI ANT_Send_Acknowledged(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Send_Acknowledged_t)(unsigned int BluetoothStackID, Byte_t ChannelNumber, Byte_t *Data);
#endif

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the Bluetooth device specified by     */
   /* BluetoothStackID.  This function accepts as it's second argument, */
   /* the sequence / channel number that the data will be sent on.  The */
   /* upper three bits of this argument are the sequence number, and the*/
   /* lower five bits are the channel number.  This function accepts as */
   /* it's third argument, a pointer to a byte array of the burst data  */
   /* to send.  The data array MUST be at least eight bytes in length,  */
   /* and only the first eight bytes will be sent.  This function       */
   /* returns zero if the Packet was successfully sent, otherwise this  */
   /* function returns a negative error code.                           */
BTPSAPI_DECLARATION int BTPSAPI ANT_Send_Burst_Transfer(unsigned int BluetoothStackID, Byte_t SequenceChannelNumber, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Send_Burst_Transfer_t)(unsigned int BluetoothStackID, Byte_t SequenceChannelNumber, Byte_t *Data);
#endif

   /* Test Mode Message API.                                            */

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode.  This function accepts as it's first argument,   */
   /* the Bluetooth device specified by BluetoothStackID.  This function*/
   /* returns zero if the Packet was successfully sent, otherwise this  */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
BTPSAPI_DECLARATION int BTPSAPI ANT_Initialize_CW_Test_Mode(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Initialize_CW_Test_Mode_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument, the     */
   /* Bluetooth device specified by BluetoothStackID.  This function    */
   /* accepts as it's second argument, the transmit power level to be   */
   /* used.  This function accepts as it's third argument, the RF       */
   /* frequency to be used.  This function returns zero if the Packet   */
   /* was successfully sent, otherwise this function returns a negative */
   /* error code.                                                       */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANT_Initialize_CW_Test_Mode.                     */
BTPSAPI_DECLARATION int BTPSAPI ANT_Set_CW_Test_Mode(unsigned int BluetoothStackID, Byte_t TxPower, Byte_t RFFrequency);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Set_CW_Test_Mode_t)(unsigned int BluetoothStackID, Byte_t TxPower, Byte_t RFFrequency);
#endif

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the        */
   /* Bluetooth device specified by BluetoothStackID.  This function    */
   /* accepts as it's second argument, the size of the packet data      */
   /* buffer. This function accepts as it's third argument, a pointer to*/
   /* a buffer containing the ANT packet data.  This function returns   */
   /* zero if the Packet was successfully sent, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet buffer should contain entire ANT packets,     */
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
BTPSAPI_DECLARATION int BTPSAPI ANT_Send_Raw_Packet(unsigned int BluetoothStackID, unsigned int DataSize, Byte_t *Packet);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANT_Send_Raw_Packet_t)(unsigned int BluetoothStackID, unsigned int DataSize, Byte_t *Packet);
#endif

#endif
