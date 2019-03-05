// Copyright (c) 2011 Qualcomm Atheros, Inc.  All rights reserved.
// $ATH_LICENSE_HW_HDR_C$
//
// DO NOT EDIT!  These definitions are tied to a particular hardware layout

#ifndef _RX_ATTENTION_H_
#define _RX_ATTENTION_H_
#if !defined(__ASSEMBLER__)
#endif

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	first_mpdu[0], last_mpdu[1], mcast_bcast[2], peer_idx_invalid[3], peer_idx_timeout[4], power_mgmt[5], non_qos[6], null_data[7], mgmt_type[8], ctrl_type[9], more_data[10], eosp[11], u_apsd_trigger[12], fragment[13], order[14], classification[15], overflow_err[16], msdu_length_err[17], tcp_udp_chksum_fail[18], ip_chksum_fail[19], sa_idx_invalid[20], da_idx_invalid[21], sa_idx_timeout[22], da_idx_timeout[23], encrypt_required[24], directed[25], buffer_fragment[26], mpdu_length_err[27], tkip_mic_err[28], decrypt_err[29], fcs_err[30], msdu_done[31]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_ATTENTION 1

struct rx_attention {
    volatile uint32_t first_mpdu                      :  1, //[0]
                      last_mpdu                       :  1, //[1]
                      mcast_bcast                     :  1, //[2]
                      peer_idx_invalid                :  1, //[3]
                      peer_idx_timeout                :  1, //[4]
                      power_mgmt                      :  1, //[5]
                      non_qos                         :  1, //[6]
                      null_data                       :  1, //[7]
                      mgmt_type                       :  1, //[8]
                      ctrl_type                       :  1, //[9]
                      more_data                       :  1, //[10]
                      eosp                            :  1, //[11]
                      u_apsd_trigger                  :  1, //[12]
                      fragment                        :  1, //[13]
                      order                           :  1, //[14]
                      classification                  :  1, //[15]
                      overflow_err                    :  1, //[16]
                      msdu_length_err                 :  1, //[17]
                      tcp_udp_chksum_fail             :  1, //[18]
                      ip_chksum_fail                  :  1, //[19]
                      sa_idx_invalid                  :  1, //[20]
                      da_idx_invalid                  :  1, //[21]
                      sa_idx_timeout                  :  1, //[22]
                      da_idx_timeout                  :  1, //[23]
                      encrypt_required                :  1, //[24]
                      directed                        :  1, //[25]
                      buffer_fragment                 :  1, //[26]
                      mpdu_length_err                 :  1, //[27]
                      tkip_mic_err                    :  1, //[28]
                      decrypt_err                     :  1, //[29]
                      fcs_err                         :  1, //[30]
                      msdu_done                       :  1; //[31]
};

/*

first_mpdu
			Indicates the first MSDU of the PPDU.  If both first_mpdu
			and last_mpdu are set in the MSDU then this is a not an
			A-MPDU frame but a stand alone MPDU.  Interior MPDU in an
			A-MPDU shall have both first_mpdu and last_mpdu bits set to
			0.  The PPDU start status will only be valid when this bit
			is set.

last_mpdu
			Indicates the last MSDU of the last MPDU of the PPDU.  The
			PPDU end status will only be valid when this bit is set.

mcast_bcast
			Multicast / broadcast indicator.  Only set when the MAC
			address 1 bit 0 is set indicating mcast/bcast and the BSSID
			matches one of the 4 BSSID registers. Only set when
			first_msdu is set.

peer_idx_invalid
			Indicates no matching entries within the the max search
			count.  Only set when first_msdu is set.

peer_idx_timeout
			Indicates an unsuccessful search for the peer index due to
			timeout.  Only set when first_msdu is set.

power_mgmt
			Power management bit set in the 802.11 header.  Only set
			when first_msdu is set.

non_qos
			Set if packet is not a non-QoS data frame.  Only set when
			first_msdu is set.

null_data
			Set if frame type indicates either null data or QoS null
			data format.  Only set when first_msdu is set.

mgmt_type
			Set if packet is a management packet.  Only set when
			first_msdu is set.

ctrl_type
			Set if packet is a control packet.  Only set when first_msdu
			is set.

more_data
			Set if more bit in frame control is set.  Only set when
			first_msdu is set.

eosp
			Set if the EOSP (end of service period) bit in the QoS
			control field is set.  Only set when first_msdu is set.

u_apsd_trigger
			Set if packet is U-APSD trigger.  Key table will have bits
			per TID to indicate U-APSD trigger.

fragment
			Indicates that this is an 802.11 fragment frame.  This is
			set when either the more_frag bit is set in the frame
			control or the fragment number is not zero.  Only set when
			first_msdu is set.

order
			Set if the order bit in the frame control is set.  Only set
			when first_msdu is set.

classification
			Indicates that this status has a corresponding MSDU that
			requires FW processing.  The OLE will have classification
			ring mask registers which will indicate the ring(s) for
			packets and descriptors which need FW attention.

overflow_err
			PCU Receive FIFO does not have enough space to store the
			full receive packet.  Enough space is reserved in the
			receive FIFO for the status is written.  This MPDU remaining
			packets in the PPDU will be filtered and no Ack response
			will be transmitted.

msdu_length_err
			Indicates that the MSDU length from the 802.3 encapsulated
			length field extends beyond the MPDU boundary.

tcp_udp_chksum_fail
			Indicates that the computed checksum (tcp_udp_chksum) did
			not match the checksum in the TCP/UDP header.

ip_chksum_fail
			Indicates that the computed checksum did not match the
			checksum in the IP header.

sa_idx_invalid
			Indicates no matching entry was found in the address search
			table for the source MAC address.

da_idx_invalid
			Indicates no matching entry was found in the address search
			table for the destination MAC address.

sa_idx_timeout
			Indicates an unsuccessful search for the source MAC address
			due to the expiring of the search timer.

da_idx_timeout
			Indicates an unsuccessful search for the destination MAC
			address due to the expiring of the search timer.

encrypt_required
			Indicates that this data type frame is not encrypted even if
			the policy for this MPDU requires encryption as indicated in
			the peer table key type.

directed
			MPDU is a directed packet which means that the RA matched
			our STA addresses.  In proxySTA it means that the TA matched
			an entry in our address search table with the corresponding
			'no_ack' bit is the address search entry cleared.

buffer_fragment
			Indicates that at least one of the rx buffers has been
			fragmented.  If set the FW should look at the rx_frag_info
			descriptor described below.

mpdu_length_err
			Indicates that the MPDU was pre-maturely terminated
			resulting in a truncated MPDU.  Don't trust the MPDU length
			field.

tkip_mic_err
			Indicates that the MPDU Michael integrity check failed

decrypt_err
			Indicates that the MPDU decrypt integrity check failed

fcs_err
			Indicates that the MPDU FCS check failed

msdu_done
			If set indicates that the RX packet data, RX header data, RX
			PPDU start descriptor, RX MPDU start/end descriptor, RX MSDU
			start/end descriptors and RX Attention descriptor are all
			valid.  This bit must be in the last octet of the
			descriptor.
*/


/* Description		RX_ATTENTION_0_FIRST_MPDU
			Indicates the first MSDU of the PPDU.  If both first_mpdu
			and last_mpdu are set in the MSDU then this is a not an
			A-MPDU frame but a stand alone MPDU.  Interior MPDU in an
			A-MPDU shall have both first_mpdu and last_mpdu bits set to
			0.  The PPDU start status will only be valid when this bit
			is set.
*/
#define RX_ATTENTION_0_FIRST_MPDU_OFFSET                             0x00000000
#define RX_ATTENTION_0_FIRST_MPDU_LSB                                0
#define RX_ATTENTION_0_FIRST_MPDU_MASK                               0x00000001

/* Description		RX_ATTENTION_0_LAST_MPDU
			Indicates the last MSDU of the last MPDU of the PPDU.  The
			PPDU end status will only be valid when this bit is set.
*/
#define RX_ATTENTION_0_LAST_MPDU_OFFSET                              0x00000000
#define RX_ATTENTION_0_LAST_MPDU_LSB                                 1
#define RX_ATTENTION_0_LAST_MPDU_MASK                                0x00000002

/* Description		RX_ATTENTION_0_MCAST_BCAST
			Multicast / broadcast indicator.  Only set when the MAC
			address 1 bit 0 is set indicating mcast/bcast and the BSSID
			matches one of the 4 BSSID registers. Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_0_MCAST_BCAST_OFFSET                            0x00000000
#define RX_ATTENTION_0_MCAST_BCAST_LSB                               2
#define RX_ATTENTION_0_MCAST_BCAST_MASK                              0x00000004

/* Description		RX_ATTENTION_0_PEER_IDX_INVALID
			Indicates no matching entries within the the max search
			count.  Only set when first_msdu is set.
*/
#define RX_ATTENTION_0_PEER_IDX_INVALID_OFFSET                       0x00000000
#define RX_ATTENTION_0_PEER_IDX_INVALID_LSB                          3
#define RX_ATTENTION_0_PEER_IDX_INVALID_MASK                         0x00000008

/* Description		RX_ATTENTION_0_PEER_IDX_TIMEOUT
			Indicates an unsuccessful search for the peer index due to
			timeout.  Only set when first_msdu is set.
*/
#define RX_ATTENTION_0_PEER_IDX_TIMEOUT_OFFSET                       0x00000000
#define RX_ATTENTION_0_PEER_IDX_TIMEOUT_LSB                          4
#define RX_ATTENTION_0_PEER_IDX_TIMEOUT_MASK                         0x00000010

/* Description		RX_ATTENTION_0_POWER_MGMT
			Power management bit set in the 802.11 header.  Only set
			when first_msdu is set.
*/
#define RX_ATTENTION_0_POWER_MGMT_OFFSET                             0x00000000
#define RX_ATTENTION_0_POWER_MGMT_LSB                                5
#define RX_ATTENTION_0_POWER_MGMT_MASK                               0x00000020

/* Description		RX_ATTENTION_0_NON_QOS
			Set if packet is not a non-QoS data frame.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_0_NON_QOS_OFFSET                                0x00000000
#define RX_ATTENTION_0_NON_QOS_LSB                                   6
#define RX_ATTENTION_0_NON_QOS_MASK                                  0x00000040

/* Description		RX_ATTENTION_0_NULL_DATA
			Set if frame type indicates either null data or QoS null
			data format.  Only set when first_msdu is set.
*/
#define RX_ATTENTION_0_NULL_DATA_OFFSET                              0x00000000
#define RX_ATTENTION_0_NULL_DATA_LSB                                 7
#define RX_ATTENTION_0_NULL_DATA_MASK                                0x00000080

/* Description		RX_ATTENTION_0_MGMT_TYPE
			Set if packet is a management packet.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_0_MGMT_TYPE_OFFSET                              0x00000000
#define RX_ATTENTION_0_MGMT_TYPE_LSB                                 8
#define RX_ATTENTION_0_MGMT_TYPE_MASK                                0x00000100

/* Description		RX_ATTENTION_0_CTRL_TYPE
			Set if packet is a control packet.  Only set when first_msdu
			is set.
*/
#define RX_ATTENTION_0_CTRL_TYPE_OFFSET                              0x00000000
#define RX_ATTENTION_0_CTRL_TYPE_LSB                                 9
#define RX_ATTENTION_0_CTRL_TYPE_MASK                                0x00000200

/* Description		RX_ATTENTION_0_MORE_DATA
			Set if more bit in frame control is set.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_0_MORE_DATA_OFFSET                              0x00000000
#define RX_ATTENTION_0_MORE_DATA_LSB                                 10
#define RX_ATTENTION_0_MORE_DATA_MASK                                0x00000400

/* Description		RX_ATTENTION_0_EOSP
			Set if the EOSP (end of service period) bit in the QoS
			control field is set.  Only set when first_msdu is set.
*/
#define RX_ATTENTION_0_EOSP_OFFSET                                   0x00000000
#define RX_ATTENTION_0_EOSP_LSB                                      11
#define RX_ATTENTION_0_EOSP_MASK                                     0x00000800

/* Description		RX_ATTENTION_0_U_APSD_TRIGGER
			Set if packet is U-APSD trigger.  Key table will have bits
			per TID to indicate U-APSD trigger.
*/
#define RX_ATTENTION_0_U_APSD_TRIGGER_OFFSET                         0x00000000
#define RX_ATTENTION_0_U_APSD_TRIGGER_LSB                            12
#define RX_ATTENTION_0_U_APSD_TRIGGER_MASK                           0x00001000

/* Description		RX_ATTENTION_0_FRAGMENT
			Indicates that this is an 802.11 fragment frame.  This is
			set when either the more_frag bit is set in the frame
			control or the fragment number is not zero.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_0_FRAGMENT_OFFSET                               0x00000000
#define RX_ATTENTION_0_FRAGMENT_LSB                                  13
#define RX_ATTENTION_0_FRAGMENT_MASK                                 0x00002000

/* Description		RX_ATTENTION_0_ORDER
			Set if the order bit in the frame control is set.  Only set
			when first_msdu is set.
*/
#define RX_ATTENTION_0_ORDER_OFFSET                                  0x00000000
#define RX_ATTENTION_0_ORDER_LSB                                     14
#define RX_ATTENTION_0_ORDER_MASK                                    0x00004000

/* Description		RX_ATTENTION_0_CLASSIFICATION
			Indicates that this status has a corresponding MSDU that
			requires FW processing.  The OLE will have classification
			ring mask registers which will indicate the ring(s) for
			packets and descriptors which need FW attention.
*/
#define RX_ATTENTION_0_CLASSIFICATION_OFFSET                         0x00000000
#define RX_ATTENTION_0_CLASSIFICATION_LSB                            15
#define RX_ATTENTION_0_CLASSIFICATION_MASK                           0x00008000

/* Description		RX_ATTENTION_0_OVERFLOW_ERR
			PCU Receive FIFO does not have enough space to store the
			full receive packet.  Enough space is reserved in the
			receive FIFO for the status is written.  This MPDU remaining
			packets in the PPDU will be filtered and no Ack response
			will be transmitted.
*/
#define RX_ATTENTION_0_OVERFLOW_ERR_OFFSET                           0x00000000
#define RX_ATTENTION_0_OVERFLOW_ERR_LSB                              16
#define RX_ATTENTION_0_OVERFLOW_ERR_MASK                             0x00010000

/* Description		RX_ATTENTION_0_MSDU_LENGTH_ERR
			Indicates that the MSDU length from the 802.3 encapsulated
			length field extends beyond the MPDU boundary.
*/
#define RX_ATTENTION_0_MSDU_LENGTH_ERR_OFFSET                        0x00000000
#define RX_ATTENTION_0_MSDU_LENGTH_ERR_LSB                           17
#define RX_ATTENTION_0_MSDU_LENGTH_ERR_MASK                          0x00020000

/* Description		RX_ATTENTION_0_TCP_UDP_CHKSUM_FAIL
			Indicates that the computed checksum (tcp_udp_chksum) did
			not match the checksum in the TCP/UDP header.
*/
#define RX_ATTENTION_0_TCP_UDP_CHKSUM_FAIL_OFFSET                    0x00000000
#define RX_ATTENTION_0_TCP_UDP_CHKSUM_FAIL_LSB                       18
#define RX_ATTENTION_0_TCP_UDP_CHKSUM_FAIL_MASK                      0x00040000

/* Description		RX_ATTENTION_0_IP_CHKSUM_FAIL
			Indicates that the computed checksum did not match the
			checksum in the IP header.
*/
#define RX_ATTENTION_0_IP_CHKSUM_FAIL_OFFSET                         0x00000000
#define RX_ATTENTION_0_IP_CHKSUM_FAIL_LSB                            19
#define RX_ATTENTION_0_IP_CHKSUM_FAIL_MASK                           0x00080000

/* Description		RX_ATTENTION_0_SA_IDX_INVALID
			Indicates no matching entry was found in the address search
			table for the source MAC address.
*/
#define RX_ATTENTION_0_SA_IDX_INVALID_OFFSET                         0x00000000
#define RX_ATTENTION_0_SA_IDX_INVALID_LSB                            20
#define RX_ATTENTION_0_SA_IDX_INVALID_MASK                           0x00100000

/* Description		RX_ATTENTION_0_DA_IDX_INVALID
			Indicates no matching entry was found in the address search
			table for the destination MAC address.
*/
#define RX_ATTENTION_0_DA_IDX_INVALID_OFFSET                         0x00000000
#define RX_ATTENTION_0_DA_IDX_INVALID_LSB                            21
#define RX_ATTENTION_0_DA_IDX_INVALID_MASK                           0x00200000

/* Description		RX_ATTENTION_0_SA_IDX_TIMEOUT
			Indicates an unsuccessful search for the source MAC address
			due to the expiring of the search timer.
*/
#define RX_ATTENTION_0_SA_IDX_TIMEOUT_OFFSET                         0x00000000
#define RX_ATTENTION_0_SA_IDX_TIMEOUT_LSB                            22
#define RX_ATTENTION_0_SA_IDX_TIMEOUT_MASK                           0x00400000

/* Description		RX_ATTENTION_0_DA_IDX_TIMEOUT
			Indicates an unsuccessful search for the destination MAC
			address due to the expiring of the search timer.
*/
#define RX_ATTENTION_0_DA_IDX_TIMEOUT_OFFSET                         0x00000000
#define RX_ATTENTION_0_DA_IDX_TIMEOUT_LSB                            23
#define RX_ATTENTION_0_DA_IDX_TIMEOUT_MASK                           0x00800000

/* Description		RX_ATTENTION_0_ENCRYPT_REQUIRED
			Indicates that this data type frame is not encrypted even if
			the policy for this MPDU requires encryption as indicated in
			the peer table key type.
*/
#define RX_ATTENTION_0_ENCRYPT_REQUIRED_OFFSET                       0x00000000
#define RX_ATTENTION_0_ENCRYPT_REQUIRED_LSB                          24
#define RX_ATTENTION_0_ENCRYPT_REQUIRED_MASK                         0x01000000

/* Description		RX_ATTENTION_0_DIRECTED
			MPDU is a directed packet which means that the RA matched
			our STA addresses.  In proxySTA it means that the TA matched
			an entry in our address search table with the corresponding
			'no_ack' bit is the address search entry cleared.
*/
#define RX_ATTENTION_0_DIRECTED_OFFSET                               0x00000000
#define RX_ATTENTION_0_DIRECTED_LSB                                  25
#define RX_ATTENTION_0_DIRECTED_MASK                                 0x02000000

/* Description		RX_ATTENTION_0_BUFFER_FRAGMENT
			Indicates that at least one of the rx buffers has been
			fragmented.  If set the FW should look at the rx_frag_info
			descriptor described below.
*/
#define RX_ATTENTION_0_BUFFER_FRAGMENT_OFFSET                        0x00000000
#define RX_ATTENTION_0_BUFFER_FRAGMENT_LSB                           26
#define RX_ATTENTION_0_BUFFER_FRAGMENT_MASK                          0x04000000

/* Description		RX_ATTENTION_0_MPDU_LENGTH_ERR
			Indicates that the MPDU was pre-maturely terminated
			resulting in a truncated MPDU.  Don't trust the MPDU length
			field.
*/
#define RX_ATTENTION_0_MPDU_LENGTH_ERR_OFFSET                        0x00000000
#define RX_ATTENTION_0_MPDU_LENGTH_ERR_LSB                           27
#define RX_ATTENTION_0_MPDU_LENGTH_ERR_MASK                          0x08000000

/* Description		RX_ATTENTION_0_TKIP_MIC_ERR
			Indicates that the MPDU Michael integrity check failed
*/
#define RX_ATTENTION_0_TKIP_MIC_ERR_OFFSET                           0x00000000
#define RX_ATTENTION_0_TKIP_MIC_ERR_LSB                              28
#define RX_ATTENTION_0_TKIP_MIC_ERR_MASK                             0x10000000

/* Description		RX_ATTENTION_0_DECRYPT_ERR
			Indicates that the MPDU decrypt integrity check failed
*/
#define RX_ATTENTION_0_DECRYPT_ERR_OFFSET                            0x00000000
#define RX_ATTENTION_0_DECRYPT_ERR_LSB                               29
#define RX_ATTENTION_0_DECRYPT_ERR_MASK                              0x20000000

/* Description		RX_ATTENTION_0_FCS_ERR
			Indicates that the MPDU FCS check failed
*/
#define RX_ATTENTION_0_FCS_ERR_OFFSET                                0x00000000
#define RX_ATTENTION_0_FCS_ERR_LSB                                   30
#define RX_ATTENTION_0_FCS_ERR_MASK                                  0x40000000

/* Description		RX_ATTENTION_0_MSDU_DONE
			If set indicates that the RX packet data, RX header data, RX
			PPDU start descriptor, RX MPDU start/end descriptor, RX MSDU
			start/end descriptors and RX Attention descriptor are all
			valid.  This bit must be in the last octet of the
			descriptor.
*/
#define RX_ATTENTION_0_MSDU_DONE_OFFSET                              0x00000000
#define RX_ATTENTION_0_MSDU_DONE_LSB                                 31
#define RX_ATTENTION_0_MSDU_DONE_MASK                                0x80000000


#endif // _RX_ATTENTION_H_
