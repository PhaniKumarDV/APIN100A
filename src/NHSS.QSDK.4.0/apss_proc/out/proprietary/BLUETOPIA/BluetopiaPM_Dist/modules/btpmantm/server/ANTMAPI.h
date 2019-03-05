/*****< antmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTMAPI - ANT+ Manager API for Stonestreet One Bluetooth Protocol Stack   */
/*            Platform Manager.                                               */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/30/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANTMAPIH__
#define __ANTMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "ANTMMSG.h"             /* BTPM ANT Manager Message Formats.         */

   /* The following structure is the structure that is used to store    */
   /* ANTM Initialization Information.                                  */
typedef struct _tagANTM_Initialization_Info_t
{
   unsigned long InitializationFlags;
} ANTM_Initialization_Info_t;

#define ANTM_INITIALIZATION_INFO_SIZE                       (sizeof(ANTM_Initialization_Info_t))

   /* The following constants define the bits which can be combined in  */
   /* the InitializationFlags field of the ANTM_Initialization_Info_t   */
   /* structure.                                                        */
#define ANTM_INITIALIZATION_FLAGS_RAW_MODE                  (ANT_INITIALIZATION_FLAGS_RAW_MODE)

   /* The following enumerated type represents the ANT+ Manager Event   */
   /* Types that are dispatched by this module to inform other modules  */
   /* of ANT+ Manager Changes.                                          */
typedef enum
{
   aetANTMStartupMessage,
   aetANTMChannelResponse,
   aetANTMChannelStatus,
   aetANTMChannelID,
   aetANTMANTVersion,
   aetANTMCapabilities,
   aetANTMBroadcastDataPacket,
   aetANTMAcknowledgedDataPacket,
   aetANTMBurstDataPacket,
   aetANTMExtendedBroadcastDataPacket,
   aetANTMExtendedAcknowledgedDataPacket,
   aetANTMExtendedBurstDataPacket,
   aetANTMRawDataPacket
} ANTM_Event_Type_t;

   /* ANT Event Data definitions.                                       */

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMStartupMessage event.    */
typedef struct _tagANTM_Startup_Message_Event_Data_t
{
   unsigned int CallbackID;
   unsigned int StartupMessage;
} ANTM_Startup_Message_Event_Data_t;

#define ANTM_STARTUP_MESSAGE_EVENT_DATA_SIZE                (sizeof(ANTM_Startup_Message_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMChannelResponse event.   */
typedef struct _tagANTM_Channel_Response_Event_Data_t
{
   unsigned int CallbackID;
   unsigned int ChannelNumber;
   unsigned int MessageID;
   unsigned int MessageCode;
} ANTM_Channel_Response_Event_Data_t;

#define ANTM_CHANNEL_RESPONSE_EVENT_DATA_SIZE               (sizeof(ANTM_Channel_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMChannelStatus event.     */
typedef struct _tagANTM_Channel_Status_Event_Data_t
{
   unsigned int CallbackID;
   unsigned int ChannelNumber;
   unsigned int ChannelStatus;
} ANTM_Channel_Status_Event_Data_t;

#define ANTM_CHANNEL_STATUS_EVENT_DATA_SIZE                 (sizeof(ANTM_Channel_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMChannelID event.         */
typedef struct _tagANTM_Channel_ID_Event_Data_t
{
   unsigned int CallbackID;
   unsigned int ChannelNumber;
   unsigned int DeviceNumber;
   unsigned int DeviceTypeID;
   unsigned int TransmissionType;
} ANTM_Channel_ID_Event_Data_t;

#define ANTM_CHANNEL_ID_EVENT_DATA_SIZE                     (sizeof(ANTM_Channel_ID_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMANTVersion event.        */
typedef struct _tagANTM_ANT_Version_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  VersionDataLength;
   Byte_t       *VersionData;
} ANTM_ANT_Version_Event_Data_t;

#define ANTM_ANT_VERSION_EVENT_DATA_SIZE                    (sizeof(ANTM_ANT_Version_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMCapabilities event.      */
typedef struct _tagANTM_Capabilities_Event_Data_t
{
   unsigned int CallbackID;
   unsigned int MaxChannels;
   unsigned int MaxNetworks;
   unsigned int StandardOptions;
   unsigned int AdvancedOptions;
   unsigned int AdvancedOptions2;
   unsigned int Reserved;
} ANTM_Capabilities_Event_Data_t;

#define ANTM_CAPABILITIES_EVENT_DATA_SIZE                   (sizeof(ANTM_Capabilities_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMBroadcastDataPacket      */
   /* event.                                                            */
typedef struct _tagANTM_Broadcast_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  ChannelNumber;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Broadcast_Data_Packet_Event_Data_t;

#define ANTM_BROADCAST_DATA_PACKET_EVENT_DATA_SIZE          (sizeof(ANTM_Broadcast_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMAcknowledgedDataPacket   */
   /* event.                                                            */
typedef struct _tagANTM_Acknowledged_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  ChannelNumber;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Acknowledged_Data_Packet_Event_Data_t;

#define ANTM_ACKNOWLEDGED_DATA_PACKET_EVENT_DATA_SIZE       (sizeof(ANTM_Acknowledged_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMBurstDataPacket event.   */
typedef struct _tagANTM_Burst_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  SequenceChannelNumber;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Burst_Data_Packet_Event_Data_t;

#define ANTM_BURST_DATA_PACKET_EVENT_DATA_SIZE              (sizeof(ANTM_Burst_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* aetANTMExtendedBroadcastDataPacket event.                         */
typedef struct _tagANTM_Extended_Broadcast_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  ChannelNumber;
   unsigned int  DeviceNumber;
   unsigned int  DeviceType;
   unsigned int  TransmissionType;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Extended_Broadcast_Data_Packet_Event_Data_t;

#define ANTM_EXTENDED_BROADCAST_DATA_PACKET_EVENT_DATA_SIZE (sizeof(ANTM_Extended_Broadcast_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* aetANTMExtendedAcknowledgedDataPacket event.                      */
typedef struct _tagANTM_Extended_Acknowledged_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  ChannelNumber;
   unsigned int  DeviceNumber;
   unsigned int  DeviceType;
   unsigned int  TransmissionType;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Extended_Acknowledged_Data_Packet_Event_Data_t;

#define ANTM_EXTENDED_ACKNOWLEDGED_DATA_PACKET_EVENT_DATA_SIZE (sizeof(ANTM_Extended_Acknowledged_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMExtendedBurstDataPacket  */
   /* event.                                                            */
typedef struct _tagANTM_Extended_Burst_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  SequenceChannelNumber;
   unsigned int  DeviceNumber;
   unsigned int  DeviceType;
   unsigned int  TransmissionType;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Extended_Burst_Data_Packet_Event_Data_t;

#define ANTM_EXTENDED_BURST_DATA_PACKET_EVENT_DATA_SIZE     (sizeof(ANTM_Extended_Burst_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANTMRawDataPacket event.     */
typedef struct _tagANTM_Raw_Data_Packet_Event_Data_t
{
   unsigned int  CallbackID;
   unsigned int  DataLength;
   Byte_t       *Data;
} ANTM_Raw_Data_Packet_Event_Data_t;

#define ANTM_RAW_DATA_PACKET_EVENT_DATA_SIZE                (sizeof(ANTM_Raw_Data_Packet_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* ANT+ Manager Event (and Event Data) of a ANT+ Manager Event.      */
typedef struct _tagANTM_Event_Data_t
{
   ANTM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      ANTM_Startup_Message_Event_Data_t                   StartupMessageEventData;
      ANTM_Channel_Response_Event_Data_t                  ChannelResponseEventData;
      ANTM_Channel_Status_Event_Data_t                    ChannelStatusEventData;
      ANTM_Channel_ID_Event_Data_t                        ChannelIDEventData;
      ANTM_ANT_Version_Event_Data_t                       ANTVersionEventData;
      ANTM_Capabilities_Event_Data_t                      CapabilitiesEventData;
      ANTM_Broadcast_Data_Packet_Event_Data_t             BroadcastDataPacketEventData;
      ANTM_Acknowledged_Data_Packet_Event_Data_t          AcknowledgedDataPacketEventData;
      ANTM_Burst_Data_Packet_Event_Data_t                 BurstDataPacketEventData;
      ANTM_Extended_Broadcast_Data_Packet_Event_Data_t    ExtendedBroadcastDataPacketEventData;
      ANTM_Extended_Acknowledged_Data_Packet_Event_Data_t ExtendedAcknowledgedDataPacketEventData;
      ANTM_Extended_Burst_Data_Packet_Event_Data_t        ExtendedBurstDataPacketEventData;
      ANTM_Raw_Data_Packet_Event_Data_t                   RawDataPacketEventData;
   } EventData;
} ANTM_Event_Data_t;

#define ANTM_EVENT_DATA_SIZE                                (sizeof(ANTM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the ANT+*/
   /* Manager dispatches an event (and the client has registered for    */
   /* events).  This function passes to the caller the ANT+ Manager     */
   /* Event and the Callback Parameter that was specified when this     */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the Event Data ONLY in the context of this callback.  If the      */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *ANTM_Event_Callback_t)(ANTM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANT+ Manager Module.  This function*/
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI ANTM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANTM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a event callback function with the ANT+       */
   /* Manager Service.  This Callback will be dispatched by the ANT+    */
   /* Manager when various ANT+ Manager Events occur.  This function    */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a ANT+ Manager Event needs to be      */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANTM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Register_Event_Callback(ANTM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Register_Event_Callback_t)(ANTM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANT+ Manager Event Callback   */
   /* (registered via a successful call to the                          */
   /* ANTM_Register_Event_Callback() function).  This function accepts  */
   /* as input the ANT+ Manager Event Callback ID (return value from    */
   /* ANTM_Register_Event_Callback() function).                         */
BTPSAPI_DECLARATION void BTPSAPI ANTM_Un_Register_Event_Callback(unsigned int ANTManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANTM_Un_Register_Event_Callback_t)(unsigned int ANTManagerCallbackID);
#endif

   /* Configuration Message API.                                        */

   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to register.  This function   */
   /* accepts as it's third argument, the channel type to be assigned to*/
   /* the channel.  This function accepts as it's fourth argument, the  */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fifth argument, the extended        */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Assign_Channel_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment);
#endif

   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to un-assign.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Un_Assign_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Un_Assign_Channel_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);
#endif

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, the channel number to configure.  The ANT   */
   /* channel must be assigned using ANTM_Assign_Channel() before       */
   /* calling this function.  This function accepts as it's third       */
   /* argument, the device number to search for on the channel.  Zero   */
   /* should be specified for this argument to scan for any device      */
   /* number.  This function accepts as it's fourth argument, the device*/
   /* type to search for on the channel.  Zero should be specified for  */
   /* this argument to scan for any device type.  This function accepts */
   /* as it's fifth argument, the transmission type to search for on the*/
   /* channel.  Zero should be specified for this argument to scan for  */
   /* any transmission type.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Channel_ID_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType);
#endif

   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel messaging period to   */
   /* set on the channel.  This function returns zero if successful,    */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Channel_Period(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessagingPeriod);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Channel_Period_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessagingPeriod);
#endif

   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument the     */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the search timeout to set on the  */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Channel_Search_Timeout_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout);
#endif

   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel frequency to set on   */
   /* the channel.  This function returns zero if successful, otherwise */
   /* this function returns a negative error code.                      */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Channel_RF_Frequency(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int RFFrequency);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Channel_RF_Frequency_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int RFFrequency);
#endif

   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, a pointer to the ANT network key  */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Network_Key(unsigned int ANTManagerCallbackID, unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Network_Key_t)(unsigned int ANTManagerCallbackID, unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey);
#endif

   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's     */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument the transmit power to set on the device.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int TransmitPower);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Transmit_Power_t)(unsigned int ANTManagerCallbackID, unsigned int TransmitPower);
#endif

   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to add to the */
   /* list.  This function accepts as it's third argument, the device   */
   /* number to add to the list.  This function accepts as it's fourth  */
   /* argument, the device type to add to the list.  This function      */
   /* accepts as it's fifth argument, the transmission type to add to   */
   /* the list.  This function accepts as it's sixth argument, the the  */
   /* list index to overwrite with the updated entry.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Add_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Add_Channel_ID_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex);
#endif

   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* on which the list should be configured.  This function accepts as */
   /* it's third argument, the size of the list.  This function accepts */
   /* as it's fourth argument, the list type.  Zero should be specified */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Configure_Inclusion_Exclusion_List(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Configure_Inclusion_Exclusion_List_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude);
#endif

   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the transmit power level for the  */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Channel_Transmit_Power(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int TransmitPower);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Channel_Transmit_Power_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int TransmitPower);
#endif

   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to   */
   /* configure.  This function accepts as it's third argument, the     */
   /* search timeout to set on the channel.  This function returns zero */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Low_Priority_Channel_Search_Timeout(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Low_Priority_Channel_Search_Timeout_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchTimeout);
#endif

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument the Callback*/
   /* ID that was returned from a successful call                       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  The ANT channel*/
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's third argument, the      */
   /* device type to search for on the channel.  Zero should be         */
   /* specified for this argument to scan for any device type.  This    */
   /* function accepts as it's fourth argument, the transmission type to*/
   /* search for on the channel.  Zero should be specified for this     */
   /* argument to scan for any transmission type.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Serial_Number_Channel_ID(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Serial_Number_Channel_ID_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument whether or not to enable extended Rx messages.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Enable_Extended_Messages(unsigned int ANTManagerCallbackID, Boolean_t Enable);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Enable_Extended_Messages_t)(unsigned int ANTManagerCallbackID, Boolean_t Enable);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, whether or not to enable the LED.  This     */
   /* function returns zero if successful, otherwise this function      */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Enable_LED(unsigned int ANTManagerCallbackID, Boolean_t Enable);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Enable_LED_t)(unsigned int ANTManagerCallbackID, Boolean_t Enable);
#endif

   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function accepts as */
   /* it's only argument the Callback ID that was returned from a       */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Enable_Crystal(unsigned int ANTManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Enable_Crystal_t)(unsigned int ANTManagerCallbackID);
#endif

   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the bitmask of extended */
   /* Rx messages that shall be enabled or disabled.  This function     */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Configure_Extended_Messages(unsigned int ANTManagerCallbackID, unsigned int EnabledExtendedMessagesMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Configure_Extended_Messages_t)(unsigned int ANTManagerCallbackID, unsigned int EnabledExtendedMessagesMask);
#endif

   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to configure. */
   /* This function accepts as it's third, fourth, and fifth arguments, */
   /* the three operating agility frequencies to set.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Configure_Frequency_Agility(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Configure_Frequency_Agility_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3);
#endif

   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Proximity_Search(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchThreshold);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Proximity_Search_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchThreshold);
#endif

   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_Channel_Search_Priority(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchPriority);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_Channel_Search_Priority_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int SearchPriority);
#endif

   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the descriptor string type to    */
   /* set.  This function accepts as it's third argument, the           */
   /* NULL-terminated descriptor string to be set.  This function       */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_USB_Descriptor_String(unsigned int ANTManagerCallbackID, unsigned int StringNumber, char *DescriptorString);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_USB_Descriptor_String_t)(unsigned int ANTManagerCallbackID, unsigned int StringNumber, char *DescriptorString);
#endif

   /* Control Message API.                                              */

   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  This function accepts as it's only     */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  A delay of at least 500ms is     */
   /* suggested after calling this function to allow time for the module*/
   /* to reset.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Reset_System(unsigned int ANTManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Reset_System_t)(unsigned int ANTManagerCallbackID);
#endif

   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  The channel    */
   /* specified must have been assigned and configured before calling   */
   /* this function.  This function returns zero if successful,         */
   /* otherwise this function returns a negative error code.            */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Open_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Open_Channel_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);
#endif

   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Close_Channel(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Close_Channel_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);
#endif

   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number that the request will be sent */
   /* to.  This function accepts as it's third argument, the message ID */
   /* being requested from the channel.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
BTPSAPI_DECLARATION int BTPSAPI ANTM_Request_Message(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessageID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Request_Message_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int MessageID);
#endif

   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  The channel specified must have been assigned and        */
   /* configured as a SLAVE Rx ONLY channel before calling this         */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Open_Rx_Scan_Mode(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Open_Rx_Scan_Mode_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber);
#endif

   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function accepts as it's only      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Sleep_Message(unsigned int ANTManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Sleep_Message_t)(unsigned int ANTManagerCallbackID);
#endif

   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be broadcast on.  This function accepts as it's     */
   /* third argument the length of the data to send.  This function     */
   /* accepts as it's fourth argument a pointer to a byte array of the  */
   /* broadcast data to send.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Send_Broadcast_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Send_Broadcast_Data_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data);
#endif

   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be sent on.  This function accepts as it's third    */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's fourth argument, a pointer to a byte array of the            */
   /* acknowledged data to send.  This function returns zero if         */
   /* successful, otherwise this function returns a negative error code.*/
BTPSAPI_DECLARATION int BTPSAPI ANTM_Send_Acknowledged_Data(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Send_Acknowledged_Data_t)(unsigned int ANTManagerCallbackID, unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data);
#endif

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the sequence / channel  */
   /* number that the data will be sent on.  The upper three bits of    */
   /* this argument are the sequence number, and the lower five bits are*/
   /* the channel number.  This function accepts as it's third argument */
   /* the length of the data to send.  This function accepts as it's    */
   /* fourth argument, a pointer to a byte array of the burst data to   */
   /* send.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Send_Burst_Transfer_Data(unsigned int ANTManagerCallbackID, unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Send_Burst_Transfer_Data_t)(unsigned int ANTManagerCallbackID, unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data);
#endif

   /* Test Mode Message API.                                            */

   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function accepts as it's only argument the */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Initialize_CW_Test_Mode(unsigned int ANTManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Initialize_CW_Test_Mode_t)(unsigned int ANTManagerCallbackID);
#endif

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument the      */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the transmit power level to be used.  This       */
   /* function accepts as it's third argument, the RF frequency to be   */
   /* used.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Set_CW_Test_Mode(unsigned int ANTManagerCallbackID, unsigned int TxPower, unsigned int RFFrequency);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Set_CW_Test_Mode_t)(unsigned int ANTManagerCallbackID, unsigned int TxPower, unsigned int RFFrequency);
#endif

   /* Raw Mode API.                                                     */

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the        */
   /* Callback ID that was returned from a successful call to           */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Send_Raw_Packet(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Send_Raw_Packet_t)(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData);
#endif

   /* The following function is responsible for sending a raw ANT       */
   /* packet without waiting for the command to be queued for sending   */
   /* to the chip.  This function accepts as it's first argument,       */
   /* the Callback ID that was returned from a successful call to       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the size of the packet data buffer.  This        */
   /* function accepts as it's third argument, a pointer to a buffer    */
   /* containing the ANT packet to be sent. This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
BTPSAPI_DECLARATION int BTPSAPI ANTM_Send_Raw_Packet_Async(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANTM_Send_Raw_Packet_Async_t)(unsigned int ANTManagerCallbackID, unsigned int DataSize, Byte_t *PacketData);
#endif

#endif
