/*****< antmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTMGR - ANT Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/30/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANTMGRH__
#define __ANTMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the ANT Manager Implementation.  The function accpets  */
   /* as a parameter the Flags with which to initialize the ANT module. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error initializing the Bluetopia       */
   /* Platform Manager ANT Manager Implementation.                      */
int _ANTM_Initialize(unsigned long InitializationFlags);

   /* The following function is responsible for shutting down the ANT   */
   /* Manager Implementation.  After this function is called the ANT    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _ANTM_Initialize() function.  */
void _ANTM_Cleanup(void);

   /* The following function is responsible for informing the ANT       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the ANT Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _ANTM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* Configuration Message API.                                        */

   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument, the channel number to register.  This function accepts  */
   /* as it's second argument, the channel type to be assigned to the   */
   /* channel.  This function accepts as it's third argument, the       */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fourth argument, the extended       */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Assign_Channel(unsigned int ChannelNumber, unsigned int ChannelType, unsigned int NetworkNumber, unsigned int ExtendedAssignment);

   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's only argument, the channel number to*/
   /* un-assign.  This function returns zero if successful, otherwise   */
   /* this function returns a negative error code.                      */
int _ANTM_Un_Assign_Channel(unsigned int ChannelNumber);

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument, the channel number to configure.  The ANT channel */
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's second argument, the     */
   /* device number to search for on the channel.  Zero should be       */
   /* specified for this argument to scan for any device number.  This  */
   /* function accepts as it's third argument, the device type to search*/
   /* for on the channel.  Zero should be specified for this argument to*/
   /* scan for any device type.  This function accepts as it's fourth   */
   /* argument, the transmission type to search for on the channel.     */
   /* Zero should be specified for this argument to scan for any        */
   /* transmission type.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
int _ANTM_Set_Channel_ID(unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType);

   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument, the channel number  */
   /* to configure.  This function accepts as it's second argument, the */
   /* channel messaging period to set on the channel.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
int _ANTM_Set_Channel_Period(unsigned int ChannelNumber, unsigned int MessagingPeriod);

   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument, the    */
   /* channel number to configure.  This function accepts as it's second*/
   /* argument, the search timeout to set on the channel.  This function*/
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
int _ANTM_Set_Channel_Search_Timeout(unsigned int ChannelNumber, unsigned int SearchTimeout);

   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument, the channel number to configure.  This function accepts */
   /* as it's second argument, the channel frequency to set on the      */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
int _ANTM_Set_Channel_RF_Frequency(unsigned int ChannelNumber, unsigned int RFFrequency);

   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument, the channel number to configure.  This function accepts */
   /* as it's second argument, a pointer to the ANT network key to set  */
   /* on the channel.  This function returns zero if successful,        */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
int _ANTM_Set_Network_Key(unsigned int NetworkNumber, ANT_Network_Key_t NetworkKey);

   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's only*/
   /* argument, the transmit power to set on the device.  This function */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
int _ANTM_Set_Transmit_Power(unsigned int TransmitPower);

   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument, the channel number to add to the list.    */
   /* This function accepts as it's second argument, the device number  */
   /* to add to the list.  This function accepts as it's third argument,*/
   /* the device type to add to the list.  This function accepts as it's*/
   /* fourth argument, the transmission type to add to the list.  This  */
   /* function accepts as it's fifth argument, the the list index to    */
   /* overwrite with the updated entry.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Add_Channel_ID(unsigned int ChannelNumber, unsigned int DeviceNumber, unsigned int DeviceType, unsigned int TransmissionType, unsigned int ListIndex);

   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument, the channel number on    */
   /* which the list should be configured.  This function accepts as    */
   /* it's second argument, the size of the list.  This function accepts*/
   /* as it's third argument, the list type.  Zero should be specified  */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Configure_Inclusion_Exclusion_List(unsigned int ChannelNumber, unsigned int ListSize, unsigned int Exclude);

   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument, the channel number to configure.  This function accepts */
   /* as it's second argument, the transmit power level for the         */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_Channel_Transmit_Power(unsigned int ChannelNumber, unsigned int TransmitPower);

   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument, the channel number to configure.  */
   /* This function accepts as it's second argument, the search timeout */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
int _ANTM_Set_Low_Priority_Channel_Search_Timeout(unsigned int ChannelNumber, unsigned int SearchTimeout);

   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument, the channel*/
   /* number to configure.  The ANT channel must be assigned using      */
   /* ANTM_Assign_Channel() before calling this function.  This function*/
   /* accepts as it's second argument, the device type to search for on */
   /* the channel.  Zero should be specified for this argument to scan  */
   /* for any device type.  This function accepts as it's third         */
   /* argument, the transmission type to search for on the channel.     */
   /* Zero should be specified for this argument to scan for any        */
   /* transmission type.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Set_Serial_Number_Channel_ID(unsigned int ChannelNumber, unsigned int DeviceType, unsigned int TransmissionType);

   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's only argument, whether or not to    */
   /* enable extended Rx messages.  This function returns zero if       */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Enable_Extended_Messages(unsigned int Enable);

   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument, whether or not to enable the LED.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
int _ANTM_Enable_LED(unsigned int Enable);

   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function returns    */
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
int _ANTM_Enable_Crystal(void);

   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument, the bitmask of extended Rx        */
   /* messages that shall be enabled or disabled.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Configure_Extended_Messages(unsigned int EnabledExtendedMessagesMask);

   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument, the channel number to configure.  This    */
   /* function accepts as it's second, third, and fourth arguments, the */
   /* three operating agility frequencies to set.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
int _ANTM_Configure_Frequency_Agility(unsigned int ChannelNumber, unsigned int FrequencyAgility1, unsigned int FrequencyAgility2, unsigned int FrequencyAgility3);

   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument, the channel number to    */
   /* configure.  This function accepts as it's second argument, the    */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
int _ANTM_Set_Proximity_Search(unsigned int ChannelNumber, unsigned int SearchThreshold);

   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument, the channel number to    */
   /* configure.  This function accepts as it's second argument, the    */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_Channel_Search_Priority(unsigned int ChannelNumber, unsigned int SearchPriority);

   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument, the descriptor string type to set.  This  */
   /* function accepts as it's second argument, the NULL-terminated     */
   /* descriptor string to be set.  This function returns zero if       */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
int _ANTM_Set_USB_Descriptor_String(unsigned int StringNumber, char *DescriptorString);

   /* Control Message API.                                              */

   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  A delay of at least 500ms is suggested */
   /* after calling this function to allow time for the module to reset.*/
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Reset_System(void);

   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument, the channel number to be opened.  The channel specified */
   /* must have been assigned and configured before calling this        */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
int _ANTM_Open_Channel(unsigned int ChannelNumber);

   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument, the channel number to be opened.  This function returns */
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
int _ANTM_Close_Channel(unsigned int ChannelNumber);

   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument, the channel number  */
   /* that the request will be sent to.  This function accepts as it's  */
   /* second argument, the message ID being requested from the channel. */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Request_Message(unsigned int ChannelNumber, unsigned int MessageID);

   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument, the channel number to be opened.  */
   /* The channel specified must have been assigned and configured as a */
   /* SLAVE Rx ONLY channel before calling this function.  This function*/
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
int _ANTM_Open_Rx_Scan_Mode(unsigned int ChannelNumber);

   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
int _ANTM_Sleep_Message(void);

   /* Data Message API.                                                 */

   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument, the channel number that the data  */
   /* will be broadcast on.  This function accepts as it's second       */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's third argument a pointer to a byte array of the broadcast    */
   /* data to send.  This function returns zero if successful, otherwise*/
   /* this function returns a negative error code.                      */
int _ANTM_Send_Broadcast_Data(unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data);

   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument, the channel number that the data  */
   /* will be sent on.  This function accepts as it's second argument   */
   /* the length of the data to send.  This function accepts as it's    */
   /* third argument, a pointer to a byte array of the acknowledged data*/
   /* to send.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
int _ANTM_Send_Acknowledged_Data(unsigned int ChannelNumber, unsigned int DataLength, Byte_t *Data);

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument, the sequence / channel number that*/
   /* the data will be sent on.  The upper three bits of this argument  */
   /* are the sequence number, and the lower five bits are the channel  */
   /* number.  This function accepts as it's second argument the length */
   /* of the data to send.  This function accepts as it's third         */
   /* argument, a pointer to a byte array of the burst data to send.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
int _ANTM_Send_Burst_Transfer_Data(unsigned int SequenceChannelNumber, unsigned int DataLength, Byte_t *Data);

   /* Test Mode Message API.                                            */

   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function returns zero if successful,       */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
int _ANTM_Initialize_CW_Test_Mode(void);

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument, the     */
   /* transmit power level to be used.  This function accepts as it's   */
   /* second argument, the RF frequency to be used.  This function      */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
int _ANTM_Set_CW_Test_Mode(unsigned int TxPower, unsigned int RFFrequency);

   /* Raw Mode API.                                                     */

   /* The following function is responsible for sending a raw ANT       */
   /* packet.  This function accepts as it's first argument, the size   */
   /* of the packet data buffer.  This function accepts as it's second  */
   /* argument, a pointer to a buffer containing the ANT packet to be   */
   /* sent. This function returns zero if successful, otherwise this    */
   /* function returns a negative error code.                           */
   /* * NOTE * This function will accept multiple packets at once and   */
   /*          attempt to include them in one command packet to the     */
   /*          baseband. The DataSize may not exceed 254 bytes (Maximum */
   /*          HCI Command parameter length minus a 2-byte header).     */
   /* * NOTE * The packet data buffer should contain entire ANT packets,*/
   /*          WITHOUT the leading Sync byte or trailing checksum byte. */
int _ANTM_Send_Raw_Packet(unsigned int DataSize, Byte_t *PacketData);

#endif
