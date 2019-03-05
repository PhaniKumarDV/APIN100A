/*
 * Copyright (c) 2008, Atheros Communications Inc. 
 * All Rights Reserved.
 * 
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * 
 */

#ifndef _DEV_ATH_AR5416REG_H
#define _DEV_ATH_AR5416REG_H

/* DMA Control and Interrupt Registers */
#define AR_CR                0x0008 // MAC Control Register - only write values of 1 have effect
#define AR_CR_RXE            0x00000004 // Receive enable
#define AR_CR_RXD            0x00000020 // Receive disable
#define AR_CR_SWI            0x00000040 // One-shot software interrupt

#define AR_RXDP              0x000C // MAC receive queue descriptor pointer

#define AR_CFG               0x0014 // MAC configuration and status register
#define AR_CFG_SWTD          0x00000001 // byteswap tx descriptor words
#define AR_CFG_SWTB          0x00000002 // byteswap tx data buffer words
#define AR_CFG_SWRD          0x00000004 // byteswap rx descriptor words
#define AR_CFG_SWRB          0x00000008 // byteswap rx data buffer words
#define AR_CFG_SWRG          0x00000010 // byteswap register access data words
#define AR_CFG_AP_ADHOC_INDICATION 0x00000020 // AP/adhoc indication (0-AP 1-Adhoc)
#define AR_CFG_PHOK          0x00000100 // PHY OK status
#define AR_CFG_CLK_GATE_DIS  0x00000400 // Clock gating disable
#define AR_CFG_EEBS          0x00000200 // EEPROM busy
#define AR_CFG_PCI_MASTER_REQ_Q_THRESH         0x00060000 // Mask of PCI core master request queue full threshold
#define AR_CFG_PCI_MASTER_REQ_Q_THRESH_S       17         // Shift for PCI core master request queue full threshold

#define AR_MIRT              0x0020 // Mac Interrupt rate threshold register
#define AR_MIRT_VAL          0x0000ffff // in uS
#define AR_MIRT_VAL_S        16

#define AR_IER               0x0024 // MAC Interrupt enable register
#define AR_IER_ENABLE        0x00000001 // Global interrupt enable
#define AR_IER_DISABLE       0x00000000 // Global interrupt disable

#define AR_TIMT              0x0028 // Mac Tx Interrupt mitigation threshold
#define AR_TIMT_LAST         0x0000ffff // Last packet threshold
#define AR_TIMT_LAST_S       0
#define AR_TIMT_FIRST        0xffff0000 // First packet threshold
#define AR_TIMT_FIRST_S      16

#define AR_RIMT              0x002C // Mac Rx Interrupt mitigation threshold
#define AR_RIMT_LAST         0x0000ffff // Last packet threshold
#define AR_RIMT_LAST_S       0
#define AR_RIMT_FIRST        0xffff0000 // First packet threshold
#define AR_RIMT_FIRST_S      16

#define AR_DMASIZE_4B        0x00000000 // DMA size 4 bytes (TXCFG + RXCFG)
#define AR_DMASIZE_8B        0x00000001 // DMA size 8 bytes
#define AR_DMASIZE_16B       0x00000002 // DMA size 16 bytes
#define AR_DMASIZE_32B       0x00000003 // DMA size 32 bytes
#define AR_DMASIZE_64B       0x00000004 // DMA size 64 bytes
#define AR_DMASIZE_128B      0x00000005 // DMA size 128 bytes
#define AR_DMASIZE_256B      0x00000006 // DMA size 256 bytes
#define AR_DMASIZE_512B      0x00000007 // DMA size 512 bytes

#define AR_TXCFG             0x0030 // MAC tx DMA size config register
#define AR_TXCFG_DMASZ_MASK  0x00000007
#define AR_TXCFG_DMASZ_4B    0
#define AR_TXCFG_DMASZ_8B    1
#define AR_TXCFG_DMASZ_16B   2
#define AR_TXCFG_DMASZ_32B   3
#define AR_TXCFG_DMASZ_64B   4
#define AR_TXCFG_DMASZ_128B  5
#define AR_TXCFG_DMASZ_256B  6
#define AR_TXCFG_DMASZ_512B  7
#define AR_FTRIG             0x000003F0 // Mask for Frame trigger level
#define AR_FTRIG_S           4          // Shift for Frame trigger level
#define AR_FTRIG_IMMED       0x00000000 // bytes in PCU TX FIFO before air
#define AR_FTRIG_64B         0x00000010 // default
#define AR_FTRIG_128B        0x00000020
#define AR_FTRIG_192B        0x00000030
#define AR_FTRIG_256B        0x00000040 // 5 bits total
#define AR_FTRIG_512B        0x00000080 // 5 bits total
#define AR_TXCFG_ADHOC_BEACON_ATIM_TX_POLICY 0x00000800

#define AR_RXCFG             0x0034 // MAC rx DMA size config register
#define AR_RXCFG_CHIRP       0x00000008 // Only double chirps
#define AR_RXCFG_ZLFDMA      0x00000010 // Enable DMA of zero-length frame
#define AR_RXCFG_DMASZ_MASK  0x00000007
#define AR_RXCFG_DMASZ_4B    0
#define AR_RXCFG_DMASZ_8B    1
#define AR_RXCFG_DMASZ_16B   2
#define AR_RXCFG_DMASZ_32B   3
#define AR_RXCFG_DMASZ_64B   4
#define AR_RXCFG_DMASZ_128B  5
#define AR_RXCFG_DMASZ_256B  6
#define AR_RXCFG_DMASZ_512B  7

#define AR_MIBC              0x0040 // MAC MIB control register
#define AR_MIBC_COW          0x00000001 // counter overflow warning
#define AR_MIBC_FMC          0x00000002 // freeze MIB counters
#define AR_MIBC_CMC          0x00000004 // clear MIB counters
#define AR_MIBC_MCS          0x00000008 // MIB counter strobe increment all

#define AR_TOPS              0x0044 // MAC timeout prescale count
#define AR_TOPS_MASK         0x0000FFFF // Mask for timeout prescale

#define AR_RXNPTO            0x0048 // MAC no frame received timeout
#define AR_RXNPTO_MASK       0x000003FF // Mask for no frame received timeout

#define AR_TXNPTO            0x004C // MAC no frame trasmitted timeout
#define AR_TXNPTO_MASK       0x000003FF // Mask for no frame transmitted timeout
#define AR_TXNPTO_QCU_MASK   0x000FFC00 // Mask indicating the set of QCUs
                                        // for which frame completions will cause
                                        // a reset of the no frame transmitted timeout

#define AR_RPGTO             0x0050 // MAC receive frame gap timeout
#define AR_RPGTO_MASK        0x000003FF // Mask for receive frame gap timeout

#define AR_RPCNT             0x0054 // MAC receive frame count limit
#define AR_RPCNT_MASK        0x0000001F // Mask for receive frame count limit

#define AR_MACMISC           0x0058 // MAC miscellaneous control/status register
#define AR_MACMISC_PCI_EXT_FORCE        0x00000010 //force msb to 10 to ahb
#define AR_MACMISC_DMA_OBS              0x000001E0 // Mask for DMA observation bus mux select
#define AR_MACMISC_DMA_OBS_S            5          // Shift for DMA observation bus mux select
#define AR_MACMISC_DMA_OBS_LINE_0       0          // Observation DMA line 0
#define AR_MACMISC_DMA_OBS_LINE_1       1          // Observation DMA line 1
#define AR_MACMISC_DMA_OBS_LINE_2       2          // Observation DMA line 2
#define AR_MACMISC_DMA_OBS_LINE_3       3          // Observation DMA line 3
#define AR_MACMISC_DMA_OBS_LINE_4       4          // Observation DMA line 4
#define AR_MACMISC_DMA_OBS_LINE_5       5          // Observation DMA line 5
#define AR_MACMISC_DMA_OBS_LINE_6       6          // Observation DMA line 6
#define AR_MACMISC_DMA_OBS_LINE_7       7          // Observation DMA line 7
#define AR_MACMISC_DMA_OBS_LINE_8       8          // Observation DMA line 8
#define AR_MACMISC_MISC_OBS             0x00000E00 // Mask for MISC observation bus mux select
#define AR_MACMISC_MISC_OBS_S           9          // Shift for MISC observation bus mux select
#define AR_MACMISC_MISC_OBS_BUS_LSB     0x00007000 // Mask for MAC observation bus mux select (lsb)
#define AR_MACMISC_MISC_OBS_BUS_LSB_S   12         // Shift for MAC observation bus mux select (lsb)
#define AR_MACMISC_MISC_OBS_BUS_MSB     0x00038000 // Mask for MAC observation bus mux select (msb)
#define AR_MACMISC_MISC_OBS_BUS_MSB_S   15         // Shift for MAC observation bus mux select (msb)
#define AR_MACMISC_MISC_OBS_BUS_1       1          // MAC observation bus mux select 

#define AR_GTXTO    0x0064 // MAC global transmit timeout
#define AR_GTXTO_TIMEOUT_COUNTER    0x0000FFFF  // Mask for timeout counter (in TUs)
#define AR_GTXTO_TIMEOUT_LIMIT      0xFFFF0000  // Mask for timeout limit (in  TUs)
#define AR_GTXTO_TIMEOUT_LIMIT_S    16      // Shift for timeout limit

#define AR_GTTM     0x0068 // MAC global transmit timeout mode
#define AR_GTTM_USEC          0x00000001 // usec strobe
#define AR_GTTM_IGNORE_IDLE   0x00000002 // ignore channel idle
#define AR_GTTM_RESET_IDLE    0x00000004 // reset counter on channel idle low
#define AR_GTTM_CST_USEC      0x00000008 // CST usec strobe

#define AR_CST         0x006C // MAC carrier sense timeout
#define AR_CST_TIMEOUT_COUNTER    0x0000FFFF  // Mask for timeout counter (in TUs)
#define AR_CST_TIMEOUT_LIMIT      0xFFFF0000  // Mask for timeout limit (in  TUs)
#define AR_CST_TIMEOUT_LIMIT_S    16      // Shift for timeout limit

#define AR_SREV_VERSION_HOWL                  0x014

#define AR_SREV_5416_V20_OR_LATER(_ah) (AR_SREV_HOWL((_ah)) || AR_SREV_OWL_20_OR_LATER(_ah))
#define AR_SREV_5416_V22_OR_LATER(_ah)    (AR_SREV_HOWL((_ah)) || AR_SREV_OWL_22_OR_LATER(_ah)) 

#ifdef AR5416_EMULATION

/* XXX - AR5416 Emulation only
 * XXX - TODO - remove when emulation complete
 */
#define AR_EMU              0x0070 // MAC - special emulation only register
#define AR_EMU_RATETHROT    0x00000001 // rate throttling (enabled = 1)
#define AR_EMU_CTL          0x00000002 // ctl channel busy (busy = 1)
#define AR_EMU_EXT          0x00000004 // ext channel busy (busy = 1)
#define AR_EMU_HALF_RATE    0x00000080 // run at half-rate for encryption
#define AR_EMU_VERSION      0xFFFFFF00 // Mask for version (read only)
#define AR_EMU_VERSION_S    8      // Shift for timeout limit

#endif //AR5416_EMULATION

/* Interrupt Status Registers */
#define AR_ISR               0x0080 // MAC Primary interrupt status register
#define AR_ISR_RXOK          0x00000001 // At least one frame received sans errors
#define AR_ISR_RXDESC        0x00000002 // Receive interrupt request
#define AR_ISR_RXERR         0x00000004 // Receive error interrupt
#define AR_ISR_RXNOPKT       0x00000008 // No frame received within timeout clock
#define AR_ISR_RXEOL         0x00000010 // Received descriptor empty interrupt
#define AR_ISR_RXORN         0x00000020 // Receive FIFO overrun interrupt
#define AR_ISR_TXOK          0x00000040 // Transmit okay interrupt
#define AR_ISR_TXDESC        0x00000080 // Transmit interrupt request
#define AR_ISR_TXERR         0x00000100 // Transmit error interrupt
#define AR_ISR_TXNOPKT       0x00000200 // No frame transmitted interrupt
#define AR_ISR_TXEOL         0x00000400 // Transmit descriptor empty interrupt
#define AR_ISR_TXURN         0x00000800 // Transmit FIFO underrun interrupt
#define AR_ISR_MIB           0x00001000 // MIB interrupt - see MIBC
#define AR_ISR_SWI           0x00002000 // Software interrupt
#define AR_ISR_RXPHY         0x00004000 // PHY receive error interrupt
#define AR_ISR_RXKCM         0x00008000 // Key-cache miss interrupt
#define AR_ISR_SWBA          0x00010000 // Software beacon alert interrupt
#define AR_ISR_BRSSI         0x00020000 // Beacon threshold interrupt
#define AR_ISR_BMISS         0x00040000 // Beacon missed interrupt
#define AR_ISR_BNR           0x00100000 // Beacon not ready interrupt
#define AR_ISR_RXCHIRP       0x00200000 // Phy received a 'chirp'
#define AR_ISR_BCNMISC       0x00800000 // In venice 'or' of TIM CABEND DTIMSYNC BCNTO CABTO DTIM bits from ISR_S2
#define AR_ISR_TIM           0x00800000 // TIM interrupt
#define AR_ISR_QCBROVF       0x02000000 // QCU CBR overflow interrupt
#define AR_ISR_QCBRURN       0x04000000 // QCU CBR underrun interrupt
#define AR_ISR_QTRIG         0x08000000 // QCU scheduling trigger interrupt
#define AR_ISR_GENTMR        0x10000000 // OR of generic timer bits in ISR 5

#define AR_ISR_TXMINTR       0x00080000 // Maximum interrupt transmit rate
#define AR_ISR_RXMINTR       0x01000000 // Maximum interrupt receive rate
#define AR_ISR_TXINTM        0x40000000 // Tx interrupt after mitigation
#define AR_ISR_RXINTM        0x80000000 // Rx interrupt after mitigation

#define AR_ISR_S0               0x0084 // MAC Secondary interrupt status register 0
#define AR_ISR_S0_QCU_TXOK      0x000003FF // Mask for TXOK (QCU 0-9)
#define AR_ISR_S0_QCU_TXOK_S    0          // Shift for TXOK (QCU 0-9)
#define AR_ISR_S0_QCU_TXDESC    0x03FF0000 // Mask for TXDESC (QCU 0-9)
#define AR_ISR_S0_QCU_TXDESC_S  16         // Shift for TXDESC (QCU 0-9)

#define AR_ISR_S1              0x0088 // MAC Secondary interrupt status register 1
#define AR_ISR_S1_QCU_TXERR    0x000003FF // Mask for TXERR (QCU 0-9)
#define AR_ISR_S1_QCU_TXERR_S  0          // Shift for TXERR (QCU 0-9)
#define AR_ISR_S1_QCU_TXEOL    0x03FF0000 // Mask for TXEOL (QCU 0-9)
#define AR_ISR_S1_QCU_TXEOL_S  16         // Shift for TXEOL (QCU 0-9)

#define AR_ISR_S2              0x008c // MAC Secondary interrupt status register 2
#define AR_ISR_S2_QCU_TXURN    0x000003FF // Mask for TXURN (QCU 0-9)
#define AR_ISR_S2_CST          0x00400000 // Carrier sense timeout
#define AR_ISR_S2_GTT          0x00800000 // Global transmit timeout
#define AR_ISR_S2_TIM          0x01000000 // TIM
#define AR_ISR_S2_CABEND       0x02000000 // CABEND
#define AR_ISR_S2_DTIMSYNC     0x04000000 // DTIMSYNC
#define AR_ISR_S2_BCNTO        0x08000000 // BCNTO
#define AR_ISR_S2_CABTO        0x10000000 // CABTO
#define AR_ISR_S2_DTIM         0x20000000 // DTIM
#define AR_ISR_S2_TSFOOR       0x40000000 // Rx TSF out of range
#define AR_ISR_S2_TBTT_TIME    0x80000000 // TBTT-referenced timer

#define AR_ISR_S3             0x0090 // MAC Secondary interrupt status register 3
#define AR_ISR_S3_QCU_QCBROVF    0x000003FF // Mask for QCBROVF (QCU 0-9)
#define AR_ISR_S3_QCU_QCBRURN    0x03FF0000 // Mask for QCBRURN (QCU 0-9)

#define AR_ISR_S4              0x0094 // MAC Secondary interrupt status register 4
#define AR_ISR_S4_QCU_QTRIG    0x000003FF // Mask for QTRIG (QCU 0-9)
#define AR_ISR_S4_RESV0        0xFFFFFC00 // Reserved

#define AR_ISR_S5                   0x0098     // MAC Secondary interrupt status register 5
#define AR_ISR_S5_GENTIMER7         0x00000080 // Mask for timer 7 trigger
#define AR_ISR_S5_TIM_TIMER         0x00000010 // TIM Timer ISR
#define AR_ISR_S5_DTIM_TIMER        0x00000020 // DTIM Timer ISR
#define AR_ISR_S5_GENTIMER_TRIG     0x0000FF80 // ISR for generic timer trigger 7-15
#define AR_ISR_S5_GENTIMER_TRIG_S   0
#define AR_ISR_S5_GENTIMER_THRESH   0xFF800000 // ISR for generic timer threshold 7-15
#define AR_ISR_S5_GENTIMER_THRESH_S 16
#define AR_ISR_S5_S                 0x00d8     // ISR_S5 shadow copy

/* Interrupt Mask Registers */
#define AR_IMR               0x00a0 // MAC Primary interrupt mask register
#define AR_IMR_RXOK          0x00000001 // At least one frame received sans errors
#define AR_IMR_RXDESC        0x00000002 // Receive interrupt request
#define AR_IMR_RXERR         0x00000004 // Receive error interrupt
#define AR_IMR_RXNOPKT       0x00000008 // No frame received within timeout clock
#define AR_IMR_RXEOL         0x00000010 // Received descriptor empty interrupt
#define AR_IMR_RXORN         0x00000020 // Receive FIFO overrun interrupt
#define AR_IMR_TXOK          0x00000040 // Transmit okay interrupt
#define AR_IMR_TXDESC        0x00000080 // Transmit interrupt request
#define AR_IMR_TXERR         0x00000100 // Transmit error interrupt
#define AR_IMR_TXNOPKT       0x00000200 // No frame transmitted interrupt
#define AR_IMR_TXEOL         0x00000400 // Transmit descriptor empty interrupt
#define AR_IMR_TXURN         0x00000800 // Transmit FIFO underrun interrupt
#define AR_IMR_MIB           0x00001000 // MIB interrupt - see MIBC
#define AR_IMR_SWI           0x00002000 // Software interrupt
#define AR_IMR_RXPHY         0x00004000 // PHY receive error interrupt
#define AR_IMR_RXKCM         0x00008000 // Key-cache miss interrupt
#define AR_IMR_SWBA          0x00010000 // Software beacon alert interrupt
#define AR_IMR_BRSSI         0x00020000 // Beacon threshold interrupt
#define AR_IMR_BMISS         0x00040000 // Beacon missed interrupt
#define AR_IMR_BNR           0x00100000 // BNR interrupt
#define AR_IMR_RXCHIRP       0x00200000 // RXCHIRP interrupt
#define AR_IMR_BCNMISC       0x00800000 // Venice: BCNMISC
#define AR_IMR_TIM           0x00800000 // TIM interrupt
#define AR_IMR_QCBROVF       0x02000000 // QCU CBR overflow interrupt
#define AR_IMR_QCBRURN       0x04000000 // QCU CBR underrun interrupt
#define AR_IMR_QTRIG         0x08000000 // QCU scheduling trigger interrupt
#define AR_IMR_GENTMR        0x10000000 // Generic timer interrupt

#define AR_IMR_TXMINTR       0x00080000 // Maximum interrupt transmit rate
#define AR_IMR_RXMINTR       0x01000000 // Maximum interrupt receive rate
#define AR_IMR_TXINTM        0x40000000 // Tx interrupt after mitigation
#define AR_IMR_RXINTM        0x80000000 // Rx interrupt after mitigation

#define AR_IMR_S0               0x00a4 // MAC Secondary interrupt mask register 0
#define AR_IMR_S0_QCU_TXOK      0x000003FF // Mask for TXOK (QCU 0-9)
#define AR_IMR_S0_QCU_TXOK_S    0          // Shift for TXOK (QCU 0-9)
#define AR_IMR_S0_QCU_TXDESC    0x03FF0000 // Mask for TXDESC (QCU 0-9)
#define AR_IMR_S0_QCU_TXDESC_S  16         // Shift for TXDESC (QCU 0-9)

#define AR_IMR_S1              0x00a8 // MAC Secondary interrupt mask register 1
#define AR_IMR_S1_QCU_TXERR    0x000003FF // Mask for TXERR (QCU 0-9)
#define AR_IMR_S1_QCU_TXERR_S  0          // Shift for TXERR (QCU 0-9)
#define AR_IMR_S1_QCU_TXEOL    0x03FF0000 // Mask for TXEOL (QCU 0-9)
#define AR_IMR_S1_QCU_TXEOL_S  16         // Shift for TXEOL (QCU 0-9)

#define AR_IMR_S2              0x00ac // MAC Secondary interrupt mask register 2
#define AR_IMR_S2_QCU_TXURN    0x000003FF // Mask for TXURN (QCU 0-9)
#define AR_IMR_S2_QCU_TXURN_S  0          // Shift for TXURN (QCU 0-9)
#define AR_IMR_S2_CST          0x00400000 // Carrier sense timeout
#define AR_IMR_S2_GTT          0x00800000 // Global transmit timeout
#define AR_IMR_S2_TIM          0x01000000 // TIM
#define AR_IMR_S2_CABEND       0x02000000 // CABEND
#define AR_IMR_S2_DTIMSYNC     0x04000000 // DTIMSYNC
#define AR_IMR_S2_BCNTO        0x08000000 // BCNTO
#define AR_IMR_S2_CABTO        0x10000000 // CABTO
#define AR_IMR_S2_DTIM         0x20000000 // DTIM
#define AR_IMR_S2_TSFOOR       0x40000000 // TSF out of range

#define AR_IMR_S3                0x00b0 // MAC Secondary interrupt mask register 3
#define AR_IMR_S3_QCU_QCBROVF    0x000003FF // Mask for QCBROVF (QCU 0-9)
#define AR_IMR_S3_QCU_QCBRURN    0x03FF0000 // Mask for QCBRURN (QCU 0-9)
#define AR_IMR_S3_QCU_QCBRURN_S  16         // Shift for QCBRURN (QCU 0-9)

#define AR_IMR_S4              0x00b4 // MAC Secondary interrupt mask register 4
#define AR_IMR_S4_QCU_QTRIG    0x000003FF // Mask for QTRIG (QCU 0-9)
#define AR_IMR_S4_RESV0        0xFFFFFC00 // Reserved

#define AR_IMR_S5              0x00b8 // MAC Secondary interrupt mask register 5
#define AR_IMR_S5_GENTIMER7    0x00000080 // Mask for timer 7 trigger
#define AR_IMR_S5_TIM_TIMER         0x00000010 // TIM Timer Mask
#define AR_IMR_S5_DTIM_TIMER        0x00000020 // DTIM Timer Mask
#define AR_IMR_S5_GENTIMER_TRIG     0x0000FF80 // Mask for generic timer trigger 7-15
#define AR_IMR_S5_GENTIMER_TRIG_S   0
#define AR_IMR_S5_GENTIMER_THRESH   0xFF800000 // Mask for generic timer threshold 7-15
#define AR_IMR_S5_GENTIMER_THRESH_S 16

/* Interrupt status registers (read-and-clear access secondary shadow copies) */
#define AR_ISR_RAC            0x00c0 // MAC Primary interrupt status register
                                     // read-and-clear access
#define AR_ISR_S0_S           0x00c4 // MAC Secondary interrupt status register 0
                                     // shadow copy
#define AR_ISR_S0_QCU_TXOK      0x000003FF // Mask for TXOK (QCU 0-9)
#define AR_ISR_S0_QCU_TXOK_S    0          // Shift for TXOK (QCU 0-9)
#define AR_ISR_S0_QCU_TXDESC    0x03FF0000 // Mask for TXDESC (QCU 0-9)
#define AR_ISR_S0_QCU_TXDESC_S  16         // Shift for TXDESC (QCU 0-9)

#define AR_ISR_S1_S           0x00c8 // MAC Secondary interrupt status register 1
                                     // shadow copy
#define AR_ISR_S1_QCU_TXERR    0x000003FF // Mask for TXERR (QCU 0-9)
#define AR_ISR_S1_QCU_TXERR_S  0          // Shift for TXERR (QCU 0-9)
#define AR_ISR_S1_QCU_TXEOL    0x03FF0000 // Mask for TXEOL (QCU 0-9)
#define AR_ISR_S1_QCU_TXEOL_S  16         // Shift for TXEOL (QCU 0-9)

#define AR_ISR_S2_S           0x00cc // MAC Secondary interrupt status register 2
                                     // shadow copy
#define AR_ISR_S3_S           0x00d0 // MAC Secondary interrupt status register 3
                                     // shadow copy
#define AR_ISR_S4_S           0x00d4 // MAC Secondary interrupt status register 4
                                     // shadow copy
#define AR_ISR_S5_S           0x00d8 // MAC Secondary interrupt status register 5
                                     // shadow copy
#define AR_DMADBG_0           0x00e0 // MAC DMA Debug Registers
#define AR_DMADBG_1           0x00e4
#define AR_DMADBG_2           0x00e8
#define AR_DMADBG_3           0x00ec
#define AR_DMADBG_4           0x00f0
#define AR_DMADBG_5           0x00f4
#define AR_DMADBG_6           0x00f8
#define AR_DMADBG_7           0x00fc

/* QCU registers */
#define AR_NUM_QCU      10     // Only use QCU 0-9 for forward QCU compatibility
#define AR_QCU_0        0x0001
#define AR_QCU_1        0x0002
#define AR_QCU_2        0x0004
#define AR_QCU_3        0x0008
#define AR_QCU_4        0x0010
#define AR_QCU_5        0x0020
#define AR_QCU_6        0x0040
#define AR_QCU_7        0x0080
#define AR_QCU_8        0x0100
#define AR_QCU_9        0x0200

#define AR_Q0_TXDP           0x0800 // MAC Transmit Queue descriptor pointer
#define AR_Q1_TXDP           0x0804 // MAC Transmit Queue descriptor pointer
#define AR_Q2_TXDP           0x0808 // MAC Transmit Queue descriptor pointer
#define AR_Q3_TXDP           0x080c // MAC Transmit Queue descriptor pointer
#define AR_Q4_TXDP           0x0810 // MAC Transmit Queue descriptor pointer
#define AR_Q5_TXDP           0x0814 // MAC Transmit Queue descriptor pointer
#define AR_Q6_TXDP           0x0818 // MAC Transmit Queue descriptor pointer
#define AR_Q7_TXDP           0x081c // MAC Transmit Queue descriptor pointer
#define AR_Q8_TXDP           0x0820 // MAC Transmit Queue descriptor pointer
#define AR_Q9_TXDP           0x0824 // MAC Transmit Queue descriptor pointer
#define AR_QTXDP(_i)    (AR_Q0_TXDP + ((_i)<<2))

#define AR_Q_TXE             0x0840 // MAC Transmit Queue enable
#define AR_Q_TXE_M           0x000003FF // Mask for TXE (QCU 0-9)

#define AR_Q_TXD             0x0880 // MAC Transmit Queue disable
#define AR_Q_TXD_M           0x000003FF // Mask for TXD (QCU 0-9)

#define AR_Q0_CBRCFG         0x08c0 // MAC CBR configuration
#define AR_Q1_CBRCFG         0x08c4 // MAC CBR configuration
#define AR_Q2_CBRCFG         0x08c8 // MAC CBR configuration
#define AR_Q3_CBRCFG         0x08cc // MAC CBR configuration
#define AR_Q4_CBRCFG         0x08d0 // MAC CBR configuration
#define AR_Q5_CBRCFG         0x08d4 // MAC CBR configuration
#define AR_Q6_CBRCFG         0x08d8 // MAC CBR configuration
#define AR_Q7_CBRCFG         0x08dc // MAC CBR configuration
#define AR_Q8_CBRCFG         0x08e0 // MAC CBR configuration
#define AR_Q9_CBRCFG         0x08e4 // MAC CBR configuration
#define AR_QCBRCFG(_i)      (AR_Q0_CBRCFG + ((_i)<<2))
#define AR_Q_CBRCFG_INTERVAL     0x00FFFFFF // Mask for CBR interval (us)
#define AR_Q_CBRCFG_INTERVAL_S   0          // Shift for CBR interval (us)
#define AR_Q_CBRCFG_OVF_THRESH   0xFF000000 // Mask for CBR overflow threshold
#define AR_Q_CBRCFG_OVF_THRESH_S 24         // Shift for CBR overflow threshold

#define AR_Q0_RDYTIMECFG         0x0900 // MAC ReadyTime configuration
#define AR_Q1_RDYTIMECFG         0x0904 // MAC ReadyTime configuration
#define AR_Q2_RDYTIMECFG         0x0908 // MAC ReadyTime configuration
#define AR_Q3_RDYTIMECFG         0x090c // MAC ReadyTime configuration
#define AR_Q4_RDYTIMECFG         0x0910 // MAC ReadyTime configuration
#define AR_Q5_RDYTIMECFG         0x0914 // MAC ReadyTime configuration
#define AR_Q6_RDYTIMECFG         0x0918 // MAC ReadyTime configuration
#define AR_Q7_RDYTIMECFG         0x091c // MAC ReadyTime configuration
#define AR_Q8_RDYTIMECFG         0x0920 // MAC ReadyTime configuration
#define AR_Q9_RDYTIMECFG         0x0924 // MAC ReadyTime configuration
#define AR_QRDYTIMECFG(_i)       (AR_Q0_RDYTIMECFG + ((_i)<<2))
#define AR_Q_RDYTIMECFG_DURATION   0x00FFFFFF // Mask for ReadyTime duration (us)
#define AR_Q_RDYTIMECFG_DURATION_S 0          // Shift for ReadyTime duration (us)
#define AR_Q_RDYTIMECFG_EN         0x01000000 // ReadyTime enable

#define AR_Q_ONESHOTARM_SC       0x0940     // MAC OneShotArm set control
#define AR_Q_ONESHOTARM_SC_M     0x000003FF // Mask for #define AR_Q_ONESHOTARM_SC (QCU 0-9)
#define AR_Q_ONESHOTARM_SC_RESV0 0xFFFFFC00 // Reserved

#define AR_Q_ONESHOTARM_CC       0x0980     // MAC OneShotArm clear control
#define AR_Q_ONESHOTARM_CC_M     0x000003FF // Mask for #define AR_Q_ONESHOTARM_CC (QCU 0-9)
#define AR_Q_ONESHOTARM_CC_RESV0 0xFFFFFC00 // Reserved

#define AR_Q0_MISC         0x09c0 // MAC Miscellaneous QCU settings
#define AR_Q1_MISC         0x09c4 // MAC Miscellaneous QCU settings
#define AR_Q2_MISC         0x09c8 // MAC Miscellaneous QCU settings
#define AR_Q3_MISC         0x09cc // MAC Miscellaneous QCU settings
#define AR_Q4_MISC         0x09d0 // MAC Miscellaneous QCU settings
#define AR_Q5_MISC         0x09d4 // MAC Miscellaneous QCU settings
#define AR_Q6_MISC         0x09d8 // MAC Miscellaneous QCU settings
#define AR_Q7_MISC         0x09dc // MAC Miscellaneous QCU settings
#define AR_Q8_MISC         0x09e0 // MAC Miscellaneous QCU settings
#define AR_Q9_MISC         0x09e4 // MAC Miscellaneous QCU settings
#define AR_QMISC(_i)       (AR_Q0_MISC + ((_i)<<2))
#define AR_Q_MISC_FSP                     0x0000000F // Mask for Frame Scheduling Policy
#define AR_Q_MISC_FSP_ASAP                0          // ASAP
#define AR_Q_MISC_FSP_CBR                 1          // CBR
#define AR_Q_MISC_FSP_DBA_GATED           2          // DMA Beacon Alert gated
#define AR_Q_MISC_FSP_TIM_GATED           3          // TIM gated
#define AR_Q_MISC_FSP_BEACON_SENT_GATED   4          // Beacon-sent-gated
#define AR_Q_MISC_FSP_BEACON_RCVD_GATED   5          // Beacon-received-gated
#define AR_Q_MISC_ONE_SHOT_EN             0x00000010 // OneShot enable
#define AR_Q_MISC_CBR_INCR_DIS1           0x00000020 // Disable CBR expired counter incr (empty q)
#define AR_Q_MISC_CBR_INCR_DIS0           0x00000040 // Disable CBR expired counter incr (empty beacon q)
#define AR_Q_MISC_BEACON_USE              0x00000080 // Beacon use indication
#define AR_Q_MISC_CBR_EXP_CNTR_LIMIT_EN   0x00000100 // CBR expired counter limit enable
#define AR_Q_MISC_RDYTIME_EXP_POLICY      0x00000200 // Enable TXE cleared on ReadyTime expired or VEOL
#define AR_Q_MISC_RESET_CBR_EXP_CTR       0x00000400 // Reset CBR expired counter
#define AR_Q_MISC_DCU_EARLY_TERM_REQ      0x00000800 // DCU frame early termination request control
#define AR_Q_MISC_RESV0                   0xFFFFF000 // Reserved

#define AR_Q0_STS         0x0a00 // MAC Miscellaneous QCU status
#define AR_Q1_STS         0x0a04 // MAC Miscellaneous QCU status
#define AR_Q2_STS         0x0a08 // MAC Miscellaneous QCU status
#define AR_Q3_STS         0x0a0c // MAC Miscellaneous QCU status
#define AR_Q4_STS         0x0a10 // MAC Miscellaneous QCU status
#define AR_Q5_STS         0x0a14 // MAC Miscellaneous QCU status
#define AR_Q6_STS         0x0a18 // MAC Miscellaneous QCU status
#define AR_Q7_STS         0x0a1c // MAC Miscellaneous QCU status
#define AR_Q8_STS         0x0a20 // MAC Miscellaneous QCU status
#define AR_Q9_STS         0x0a24 // MAC Miscellaneous QCU status
#define AR_QSTS(_i)       (AR_Q0_STS + ((_i)<<2))
#define AR_Q_STS_PEND_FR_CNT          0x00000003 // Mask for Pending Frame Count
#define AR_Q_STS_RESV0                0x000000FC // Reserved
#define AR_Q_STS_CBR_EXP_CNT          0x0000FF00 // Mask for CBR expired counter
#define AR_Q_STS_RESV1                0xFFFF0000 // Reserved

#define AR_Q_RDYTIMESHDN                0x0a40  /* MAC ReadyTimeShutdown status */
#define AR_Q_CBBS                       0xb00   /* Compression buffer base select */
#define AR_Q_CBBA                       0xb04   /* Compression buffer base access */
#define AR_Q_CBC                        0xb08   /* Compression buffer configuration */

#define AR_Q_RDYTIMESHDN    0x0a40     // MAC ReadyTimeShutdown status
#define AR_Q_RDYTIMESHDN_M  0x000003FF // Mask for ReadyTimeShutdown status (QCU 0-9)

/* DCU registers */
#define AR_NUM_DCU      10     // Only use 10 DCU's for forward QCU/DCU compatibility
#define AR_DCU_0        0x0001
#define AR_DCU_1        0x0002
#define AR_DCU_2        0x0004
#define AR_DCU_3        0x0008
#define AR_DCU_4        0x0010
#define AR_DCU_5        0x0020
#define AR_DCU_6        0x0040
#define AR_DCU_7        0x0080
#define AR_DCU_8        0x0100
#define AR_DCU_9        0x0200

#define AR_D0_QCUMASK     0x1000 // MAC QCU Mask
#define AR_D1_QCUMASK     0x1004 // MAC QCU Mask
#define AR_D2_QCUMASK     0x1008 // MAC QCU Mask
#define AR_D3_QCUMASK     0x100c // MAC QCU Mask
#define AR_D4_QCUMASK     0x1010 // MAC QCU Mask
#define AR_D5_QCUMASK     0x1014 // MAC QCU Mask
#define AR_D6_QCUMASK     0x1018 // MAC QCU Mask
#define AR_D7_QCUMASK     0x101c // MAC QCU Mask
#define AR_D8_QCUMASK     0x1020 // MAC QCU Mask
#define AR_D9_QCUMASK     0x1024 // MAC QCU Mask
#define AR_DQCUMASK(_i)   (AR_D0_QCUMASK + ((_i)<<2))
#define AR_D_QCUMASK         0x000003FF // Mask for QCU Mask (QCU 0-9)
#define AR_D_QCUMASK_RESV0   0xFFFFFC00 // Reserved

#define AR_D_TXBLK_CMD  0x1038      /* DCU transmit filter cmd (w/only) */
#define AR_D_TXBLK_DATA(i) (AR_D_TXBLK_CMD+(i)) /* DCU transmit filter data */

#define AR_D0_LCL_IFS     0x1040 // MAC DCU-specific IFS settings
#define AR_D1_LCL_IFS     0x1044 // MAC DCU-specific IFS settings
#define AR_D2_LCL_IFS     0x1048 // MAC DCU-specific IFS settings
#define AR_D3_LCL_IFS     0x104c // MAC DCU-specific IFS settings
#define AR_D4_LCL_IFS     0x1050 // MAC DCU-specific IFS settings
#define AR_D5_LCL_IFS     0x1054 // MAC DCU-specific IFS settings
#define AR_D6_LCL_IFS     0x1058 // MAC DCU-specific IFS settings
#define AR_D7_LCL_IFS     0x105c // MAC DCU-specific IFS settings
#define AR_D8_LCL_IFS     0x1060 // MAC DCU-specific IFS settings
#define AR_D9_LCL_IFS     0x1064 // MAC DCU-specific IFS settings
#define AR_DLCL_IFS(_i)   (AR_D0_LCL_IFS + ((_i)<<2))
#define AR_D_LCL_IFS_CWMIN       0x000003FF // Mask for CW_MIN
#define AR_D_LCL_IFS_CWMIN_S     0          // Shift for CW_MIN
#define AR_D_LCL_IFS_CWMAX       0x000FFC00 // Mask for CW_MAX
#define AR_D_LCL_IFS_CWMAX_S     10         // Shift for CW_MAX
#define AR_D_LCL_IFS_AIFS        0x0FF00000 // Mask for AIFS
#define AR_D_LCL_IFS_AIFS_S      20         // Shift for AIFS
    /*
     *  Note:  even though this field is 8 bits wide the
     *  maximum supported AIFS value is 0xfc.  Setting the AIFS value
     *  to 0xfd 0xfe or 0xff will not work correctly and will cause
     *  the DCU to hang.
     */
#define AR_D_LCL_IFS_RESV0    0xF0000000 // Reserved

#define AR_D0_RETRY_LIMIT     0x1080 // MAC Retry limits
#define AR_D1_RETRY_LIMIT     0x1084 // MAC Retry limits
#define AR_D2_RETRY_LIMIT     0x1088 // MAC Retry limits
#define AR_D3_RETRY_LIMIT     0x108c // MAC Retry limits
#define AR_D4_RETRY_LIMIT     0x1090 // MAC Retry limits
#define AR_D5_RETRY_LIMIT     0x1094 // MAC Retry limits
#define AR_D6_RETRY_LIMIT     0x1098 // MAC Retry limits
#define AR_D7_RETRY_LIMIT     0x109c // MAC Retry limits
#define AR_D8_RETRY_LIMIT     0x10a0 // MAC Retry limits
#define AR_D9_RETRY_LIMIT     0x10a4 // MAC Retry limits
#define AR_DRETRY_LIMIT(_i)   (AR_D0_RETRY_LIMIT + ((_i)<<2))
#define AR_D_RETRY_LIMIT_FR_SH       0x0000000F // Mask for frame short retry limit
#define AR_D_RETRY_LIMIT_FR_SH_S     0          // Shift for frame short retry limit
#define AR_D_RETRY_LIMIT_STA_SH      0x00003F00 // Mask for station short retry limit
#define AR_D_RETRY_LIMIT_STA_SH_S    8          // Shift for station short retry limit
#define AR_D_RETRY_LIMIT_STA_LG      0x000FC000 // Mask for station short retry limit
#define AR_D_RETRY_LIMIT_STA_LG_S    14         // Shift for station short retry limit
#define AR_D_RETRY_LIMIT_RESV0       0xFFF00000 // Reserved

#define AR_D0_CHNTIME     0x10c0 // MAC ChannelTime settings
#define AR_D1_CHNTIME     0x10c4 // MAC ChannelTime settings
#define AR_D2_CHNTIME     0x10c8 // MAC ChannelTime settings
#define AR_D3_CHNTIME     0x10cc // MAC ChannelTime settings
#define AR_D4_CHNTIME     0x10d0 // MAC ChannelTime settings
#define AR_D5_CHNTIME     0x10d4 // MAC ChannelTime settings
#define AR_D6_CHNTIME     0x10d8 // MAC ChannelTime settings
#define AR_D7_CHNTIME     0x10dc // MAC ChannelTime settings
#define AR_D8_CHNTIME     0x10e0 // MAC ChannelTime settings
#define AR_D9_CHNTIME     0x10e4 // MAC ChannelTime settings
#define AR_DCHNTIME(_i)   (AR_D0_CHNTIME + ((_i)<<2))
#define AR_D_CHNTIME_DUR         0x000FFFFF // Mask for ChannelTime duration (us)
#define AR_D_CHNTIME_DUR_S       0          // Shift for ChannelTime duration (us)
#define AR_D_CHNTIME_EN          0x00100000 // ChannelTime enable
#define AR_D_CHNTIME_RESV0       0xFFE00000 // Reserved

#define AR_D0_MISC        0x1100 // MAC Miscellaneous DCU-specific settings
#define AR_D1_MISC        0x1104 // MAC Miscellaneous DCU-specific settings
#define AR_D2_MISC        0x1108 // MAC Miscellaneous DCU-specific settings
#define AR_D3_MISC        0x110c // MAC Miscellaneous DCU-specific settings
#define AR_D4_MISC        0x1110 // MAC Miscellaneous DCU-specific settings
#define AR_D5_MISC        0x1114 // MAC Miscellaneous DCU-specific settings
#define AR_D6_MISC        0x1118 // MAC Miscellaneous DCU-specific settings
#define AR_D7_MISC        0x111c // MAC Miscellaneous DCU-specific settings
#define AR_D8_MISC        0x1120 // MAC Miscellaneous DCU-specific settings
#define AR_D9_MISC        0x1124 // MAC Miscellaneous DCU-specific settings
#define AR_DMISC(_i)      (AR_D0_MISC + ((_i)<<2))
#define AR_D_MISC_BKOFF_THRESH        0x0000003F // Mask for Backoff threshold setting
#define AR_D_MISC_RETRY_CNT_RESET_EN  0x00000040 // End of tx series station RTS/data failure count reset policy
#define AR_D_MISC_CW_RESET_EN         0x00000080 // End of tx series CW reset enable
#define AR_D_MISC_FRAG_WAIT_EN        0x00000100 // Fragment Starvation Policy
#define AR_D_MISC_FRAG_BKOFF_EN       0x00000200 // Backoff during a frag burst
#define AR_D_MISC_CW_BKOFF_EN         0x00001000 // Use binary exponential CW backoff
#define AR_D_MISC_VIR_COL_HANDLING    0x0000C000 // Mask for Virtual collision handling policy
#define AR_D_MISC_VIR_COL_HANDLING_S  14         // Shift for Virtual collision handling policy
#define AR_D_MISC_VIR_COL_HANDLING_DEFAULT 0     // Normal
#define AR_D_MISC_VIR_COL_HANDLING_IGNORE  1     // Ignore
#define AR_D_MISC_BEACON_USE          0x00010000 // Beacon use indication
#define AR_D_MISC_ARB_LOCKOUT_CNTRL   0x00060000 // Mask for DCU arbiter lockout control
#define AR_D_MISC_ARB_LOCKOUT_CNTRL_S 17         // Shift for DCU arbiter lockout control
#define AR_D_MISC_ARB_LOCKOUT_CNTRL_NONE     0   // No lockout
#define AR_D_MISC_ARB_LOCKOUT_CNTRL_INTRA_FR 1   // Intra-frame
#define AR_D_MISC_ARB_LOCKOUT_CNTRL_GLOBAL   2   // Global
#define AR_D_MISC_ARB_LOCKOUT_IGNORE  0x00080000 // DCU arbiter lockout ignore control
#define AR_D_MISC_SEQ_NUM_INCR_DIS    0x00100000 // Sequence number increment disable
#define AR_D_MISC_POST_FR_BKOFF_DIS   0x00200000 // Post-frame backoff disable
#define AR_D_MISC_VIT_COL_CW_BKOFF_EN 0x00400000 // Virtual coll. handling policy
#define AR_D_MISC_BLOWN_IFS_RETRY_EN  0x00800000 // Initiate Retry procedure on Blown IFS
#define AR_D_MISC_RESV0               0xFF000000 // Reserved

#define AR_D_SEQNUM      0x1140 // MAC Frame sequence number control/status

#define AR_D_GBL_IFS_SIFS         0x1030 // MAC DCU-global IFS settings: SIFS duration
#define AR_D_GBL_IFS_SIFS_M       0x0000FFFF // Mask for SIFS duration (core clocks)
#define AR_D_GBL_IFS_SIFS_ASYNC_FIFO_DUR 0x000003AB // SIFS duration in Async FIFO mode(core clocks)
#define AR_D_GBL_IFS_SIFS_RESV0   0xFFFFFFFF // Reserved

#define AR_D_TXBLK_BASE            0x1038 // MAC DCU-global transmit filter bits
#define AR_D_TXBLK_WRITE_BITMASK    0x0000FFFF  // Mask for bitmask
#define AR_D_TXBLK_WRITE_BITMASK_S  0       // Shift for bitmask
#define AR_D_TXBLK_WRITE_SLICE      0x000F0000  // Mask for slice
#define AR_D_TXBLK_WRITE_SLICE_S    16      // Shift for slice
#define AR_D_TXBLK_WRITE_DCU        0x00F00000  // Mask for DCU number
#define AR_D_TXBLK_WRITE_DCU_S      20      // Shift for DCU number
#define AR_D_TXBLK_WRITE_COMMAND    0x0F000000  // Mask for command
#define AR_D_TXBLK_WRITE_COMMAND_S      24      // Shift for command

#define AR_D_GBL_IFS_SLOT         0x1070 // MAC DCU-global IFS settings: slot duration
#define AR_D_GBL_IFS_SLOT_M       0x0000FFFF // Mask for Slot duration (core clocks)
#define AR_D_GBL_IFS_SLOT_RESV0   0xFFFF0000 // Reserved
#define AR_D_GBL_IFS_SLOT_ASYNC_FIFO_DUR   0x00000420 // Slot duration in Async FIFO mode(core clocks)

#define AR_D_GBL_IFS_EIFS         0x10b0 // MAC DCU-global IFS settings: EIFS duration
#define AR_D_GBL_IFS_EIFS_M       0x0000FFFF // Mask for Slot duration (core clocks)
#define AR_D_GBL_IFS_EIFS_RESV0   0xFFFF0000 // Reserved
#define AR_D_GBL_IFS_EIFS_ASYNC_FIFO_DUR   0x0000A5EB // EIFS Slot duration in Async FIFO mode

#define AR_D_GBL_IFS_MISC        0x10f0 // MAC DCU-global IFS settings: Miscellaneous
#define AR_D_GBL_IFS_MISC_LFSR_SLICE_SEL        0x00000007 // Mask forLFSR slice select
#define AR_D_GBL_IFS_MISC_TURBO_MODE            0x00000008 // Turbo mode indication
#define AR_D_GBL_IFS_MISC_USEC_DURATION         0x000FFC00 // Mask for microsecond duration
#define AR_D_GBL_IFS_MISC_DCU_ARBITER_DLY       0x00300000 // Mask for DCU arbiter delay
#define AR_D_GBL_IFS_MISC_RANDOM_LFSR_SLICE_DIS 0x01000000 // Random LSFR slice disable
#define AR_D_GBL_IFS_MISC_SLOT_XMIT_WIND_LEN    0x06000000 // Slot transmission window length mask
#define AR_D_GBL_IFS_MISC_FORCE_XMIT_SLOT_BOUND 0x08000000 // Force transmission on slot boundaries
#define AR_D_GBL_IFS_MISC_IGNORE_BACKOFF        0x10000000 // Ignore backoff

#define AR_D_FPCTL                  0x1230         // DCU frame prefetch settings
#define AR_D_FPCTL_DCU              0x0000000F   // Mask for DCU for which prefetch is enabled
#define AR_D_FPCTL_DCU_S            0            // Shift for DCU for which prefetch is enabled
#define AR_D_FPCTL_PREFETCH_EN      0x00000010   // Enable prefetch for normal (non-burst) operation
#define AR_D_FPCTL_BURST_PREFETCH   0x00007FE0   // Mask for Burst frame prefetch per DCU
#define AR_D_FPCTL_BURST_PREFETCH_S 5            // Shift for Burst frame prefetch per DCU

#define AR_D_TXPSE                 0x1270 // MAC DCU transmit pause control/status
#define AR_D_TXPSE_CTRL            0x000003FF // Mask of DCUs to pause (DCUs 0-9)
#define AR_D_TXPSE_RESV0           0x0000FC00 // Reserved
#define AR_D_TXPSE_STATUS          0x00010000 // Transmit pause status
#define AR_D_TXPSE_RESV1           0xFFFE0000 // Reserved

#define AR_D_TXSLOTMASK            0x12f0 // MAC DCU transmission slot mask
#define AR_D_TXSLOTMASK_NUM        0x0000000F // slot numbers

#define AR_CFG_LED                     0x1f04 /* LED control */
#define AR_CFG_SCLK_RATE_IND           0x00000003 /* sleep clock indication */
#define AR_CFG_SCLK_RATE_IND_S         0
#define AR_CFG_SCLK_32MHZ              0x00000000 /* Sleep clock rate */
#define AR_CFG_SCLK_4MHZ               0x00000001 /* Sleep clock rate */
#define AR_CFG_SCLK_1MHZ               0x00000002 /* Sleep clock rate */
#define AR_CFG_SCLK_32KHZ              0x00000003 /* Sleep clock rate */
#define AR_CFG_LED_BLINK_SLOW          0x00000008 /* LED slowest blink rate mode */
#define AR_CFG_LED_BLINK_THRESH_SEL    0x00000070 /* LED blink threshold select */
#define AR_CFG_LED_MODE_SEL            0x00000380 /* LED mode: bits 7..9 */
#define AR_CFG_LED_MODE_SEL_S          7          /* LED mode: bits 7..9 */
#define AR_CFG_LED_POWER               0x00000280 /* Power LED: bit 9=1, bit 7=<LED State> */
#define AR_CFG_LED_POWER_S             7          /* LED mode: bits 7..9 */
#define AR_CFG_LED_NETWORK             0x00000300 /* Network LED: bit 9=1, bit 8=<LED State> */
#define AR_CFG_LED_NETWORK_S           7          /* LED mode: bits 7..9 */
#define AR_CFG_LED_MODE_PROP           0x0        /* Blink prop to filtered tx/rx */
#define AR_CFG_LED_MODE_RPROP          0x1        /* Blink prop to unfiltered tx/rx */
#define AR_CFG_LED_MODE_SPLIT          0x2        /* Blink power for tx/net for rx */
#define AR_CFG_LED_MODE_RAND           0x3        /* Blink randomly */
#define AR_CFG_LED_MODE_POWER_OFF      0x4        /* Power LED OFF */
#define AR_CFG_LED_MODE_POWER_ON       0x5        /* Power LED ON   */
#define AR_CFG_LED_MODE_NETWORK_OFF    0x4        /* Network LED OFF */
#define AR_CFG_LED_MODE_NETWORK_ON     0x6        /* Network LED ON  */
#define AR_CFG_LED_ASSOC_CTL           0x00000c00 /* LED control: bits 10..11 */
#define AR_CFG_LED_ASSOC_CTL_S         10         /* LED control: bits 10..11 */
#define AR_CFG_LED_ASSOC_NONE          0x0        /* 0x00000000: STA is not associated or trying */
#define AR_CFG_LED_ASSOC_ACTIVE        0x1        /* 0x00000400: STA is associated */
#define AR_CFG_LED_ASSOC_PENDING       0x2        /* 0x00000800: STA is trying to associate */

#define AR_CFG_LED_BLINK_SLOW          0x00000008 /* LED slowest blink rate mode: bit 3 */
#define AR_CFG_LED_BLINK_SLOW_S        3          /* LED slowest blink rate mode: bit 3 */

#define AR_CFG_LED_BLINK_THRESH_SEL    0x00000070 /* LED blink threshold select: bits 4..6 */
#define AR_CFG_LED_BLINK_THRESH_SEL_S  4          /* LED blink threshold select: bits 4..6 */

#define AR_MAC_SLEEP                0x1f00
#define AR_MAC_SLEEP_MAC_AWAKE      0x00000000 // mac is now awake
#define AR_MAC_SLEEP_MAC_ASLEEP     0x00000001 // mac is now asleep

// DMA & PCI Registers in PCI space (usable during sleep)
#define AR_RC                0x4000 // Warm reset control register
#define AR_RC_AHB            0x00000001 // ahb reset
#define AR_RC_APB            0x00000002 // apb reset
#define AR_RC_HOSTIF         0x00000100 // host interface reset

#define AR_WA                0x4004 // PCI express work-arounds
#define AR_WA_BIT6           (1 << 6)
#define AR_WA_BIT7           (1 << 7)
#define AR_WA_BIT22          (1 << 22)
#define AR_WA_BIT23          (1 << 23)
#define AR_WA_D3_L1_DISABLE     (1 << 14)   /* Disable L1 in D3 state */
#define AR_WA_UNTIE_RESET_EN    (1 << 15)   /* Enable PCI Reset to POR (power-on-reset) */
#define AR_WA_RESET_EN          (1 << 18)   /* Sw Control to enable PCI-Reset to POR (bit 15) */
#define AR_WA_ANALOG_SHIFT      (1 << 20)
#define AR_WA_POR_SHORT         (1 << 21)   /* PCI-E Phy reset control */


#define AR9285_WA_DEFAULT          0x004a050b  /* Default value for Kite */
/*
 * 0x4004 Bit [7:6] has a known issue of hardlocking system in S3/S4/S5/restart/reboot.
 * It is found again that Kite/Kiwi is not set correctly and causing the same issue seen on OWL.
 * Must set these two bits to "0". See bug EV#61659 for example.
 */
/*
 * For Merlin chips, bit 22 of 0x4004 needs to be set to work around
 * a card disappearance issue. See bug# 32141.
 
 **** UPDATED based on EV 52795. 
 **** Merlin Specific.
 **** 1) Bit 22 does exist in Merlin 2.1. 
 **** 2) Setting bit 2 enables a PCI-E workaround in HW. With the Nvidia 9400 Chipset, this causes the chip 
 ****     to stop responding to L1 requests from the host and hangs the system. Zeroing this bit fixes the problem.
 ****     Changing value from 0x0040073f to 0x0040073b.
 */
#define AR9280_WA_DEFAULT           0x0040073b /* Default value of Workaround reg for */
                                                /* Merlin chip which includes the bit 22=1 */
                                                /* to workaround chip disappearance issue */
                                                /* See bug# 32141. */
#define AR_WA_DEFAULT               0x0000073f  /* Default value of this register */

#define AR_PM_STATE                 0x4008 // power management state
#define AR_PM_STATE_PME_D3COLD_VAUX 0x00100000 //for wow

#define AR_HOST_TIMEOUT             0x4018 // dma xfer timeout
#define AR_HOST_TIMEOUT_APB_CNTR    0x0000FFFF
#define AR_HOST_TIMEOUT_APB_CNTR_S  0
#define AR_HOST_TIMEOUT_LCL_CNTR    0xFFFF0000
#define AR_HOST_TIMEOUT_LCL_CNTR_S  16

#define AR_EEPROM                0x401c // eeprom info
#define AR_EEPROM_ABSENT         0x00000100
#define AR_EEPROM_CORRUPT        0x00000200
#define AR_EEPROM_PROT_MASK      0x03FFFC00
#define AR_EEPROM_PROT_MASK_S    10

// Protect Bits RP is read protect WP is write protect
#define EEPROM_PROTECT_RP_0_31        0x0001
#define EEPROM_PROTECT_WP_0_31        0x0002
#define EEPROM_PROTECT_RP_32_63       0x0004
#define EEPROM_PROTECT_WP_32_63       0x0008
#define EEPROM_PROTECT_RP_64_127      0x0010
#define EEPROM_PROTECT_WP_64_127      0x0020
#define EEPROM_PROTECT_RP_128_191     0x0040
#define EEPROM_PROTECT_WP_128_191     0x0080
#define EEPROM_PROTECT_RP_192_255     0x0100
#define EEPROM_PROTECT_WP_192_255     0x0200
#define EEPROM_PROTECT_RP_256_511     0x0400
#define EEPROM_PROTECT_WP_256_511     0x0800
#define EEPROM_PROTECT_RP_512_1023    0x1000
#define EEPROM_PROTECT_WP_512_1023    0x2000
#define EEPROM_PROTECT_RP_1024_2047   0x4000
#define EEPROM_PROTECT_WP_1024_2047   0x8000

#ifdef AR9100
#define AR_SREV                               0x0600 /*mac silicon rev (expanded from 8 bits to 16 bits for Sowl) */
#define AR_SREV_ID                            0x00000FFF /* Mask to read SREV info */
#else
#define AR_SREV                               0x4020 // mac silicon rev
#define AR_SREV_ID                            0x000000FF /* Mask to read SREV info */
#endif
#define AR_SREV_VERSION                       0x000000F0 /* Mask for Chip version */
#define AR_SREV_VERSION_S                     4          /* Mask to shift Major Rev Info */
#define AR_SREV_REVISION                      0x00000007 /* Mask for Chip revision level */
/* Sowl extension to SREV. AR_SREV_ID must be 0xFF */
#define AR_SREV_ID2                           0xFFFFFFFF /* Mask to read SREV info */
#define AR_SREV_VERSION2                  0xFFFC0000 /* Mask for Chip version */
#define AR_SREV_VERSION2_S                    18         /* Mask to shift Major Rev Info */
#define AR_SREV_TYPE2                            0x0003F000 /* Mask for Chip type */
#define AR_SREV_TYPE2_S                       12         /* Mask to shift Major Rev Info */
#define AR_SREV_TYPE2_CHAIN              0x00001000 /* chain mode (1 = 3 chains, 0 = 2 chains) */
#define AR_SREV_TYPE2_HOST_MODE              0x00002000 /* host mode (1 = PCI, 0 = PCIe) */
#define AR_SREV_REVISION2                  0x00000F00
#define AR_SREV_REVISION2_S               8

#define AR_SREV_VERSION_OWL_PCI               0xD
#define AR_SREV_VERSION_OWL_PCIE              0xC

#define AR_SREV_REVISION_OWL_10               0      /* Owl 1.0 */
#define AR_SREV_REVISION_OWL_20               1      /* Owl 2.0/2.1 */
#define AR_SREV_REVISION_OWL_22               2      /* Owl 2.2 */
#define AR_SREV_VERSION_SOWL                  0x40
#define AR_SREV_REVISION_SOWL_10              0      /* Sowl 1.0 */
#define AR_SREV_REVISION_SOWL_11              1      /* Sowl 1.1 */
#define AR_SREV_VERSION_MERLIN                0x80   /* Merlin Version */
#define AR_SREV_REVISION_MERLIN_10            0      /* Merlin 1.0 */
#define AR_SREV_REVISION_MERLIN_20            1      /* Merlin 2.0 */
#define AR_SREV_REVISION_MERLIN_21            2      /* Merlin 2.1 */
#define AR_SREV_VERSION_KITE                  0xC0   /* Kite Version */
#define AR_SREV_REVISION_KITE_10              0      /* Kite 1.0 */
#define AR_SREV_REVISION_KITE_11              1      /* Kite 1.1 */
#define AR_SREV_REVISION_KITE_12              2      /* Kite 1.2 */
#define AR_SREV_VERSION_KIWI                  0x180  /* Kiwi Version */
#define AR_SREV_REVISION_KIWI_10              0      /* Kiwi 1.0 */
#define AR_SREV_REVISION_KIWI_11              1      /* Kiwi 1.1 */
#define AR_SREV_REVISION_KIWI_12              2      /* Kiwi 1.2 */
#define AR_SREV_REVISION_KIWI_13              3      /* Kiwi 1.3 */
#define AR_SREV_VERSION_K2                    0x140  /* K2 Version */
#define AR_SREV_REVISION_K2_10                0      /* K2 1.0 */
#define AR_SREV_REVISION_K2_11                1      /* K2 1.1 */

#if defined(AH_SUPPORT_OWL)
/*
 * The code is compiled to support Owl.
 * Check the stored silicon rev to see if the HW is a Owl chip.
 */
#define AR_SREV_OWL(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_OWL_PCI) || \
     (AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_OWL_PCIE))
#define AR_SREV_OWL_10(_ah) \
    ((AH_PRIVATE(_ah)->ah_mac_version == AR_SREV_VERSION_OWL_PCI) || \
    (AH_PRIVATE(_ah)->ah_mac_version == AR_SREV_VERSION_OWL_PCIE))
#else
/*
 * The code is not compiled to support Owl.
 * To reduce code size, don't even check the stored silicon revision.
 */
#define AR_SREV_OWL(_ah) 0
#define AR_SREV_OWL_10(_ah) 0
#endif

#define AR_SREV_OWL_20_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version >= AR_SREV_VERSION_SOWL) || \
     (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_OWL_20))
#define AR_SREV_OWL_22_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version >= AR_SREV_VERSION_SOWL) || \
     (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_OWL_22))

#if defined(AH_SUPPORT_SOWL)
/*
 * The code is compiled to support Sowl.
 * Check the stored silicon rev to see if the HW is a Sowl chip.
 */
#define AR_SREV_SOWL(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_SOWL))
#define AR_SREV_SOWL_11(_ah) \
    (AR_SREV_SOWL(_ah) && \
     (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_SOWL_11))
#else
/*
 * The code is not compiled to support Sowl.
 * To reduce code size, don't even check the stored silicon revision.
 */
#define AR_SREV_SOWL(_ah) 0
#define AR_SREV_SOWL_11(_ah) 0
#endif

#define AR_SREV_SOWL_10_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version >= AR_SREV_VERSION_SOWL))

#if defined(AH_SUPPORT_MERLIN_ANY)
/*
 * The code is compiled to support Merlin.
 * Check the stored silicon rev to see if the HW is a Merlin chip.
 */
#define AR_SREV_MERLIN(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_MERLIN))
#define AR_SREV_MERLIN_20(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_MERLIN) && \
     (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_MERLIN_20))
#else
/*
 * The code is not compiled to support Merlin.
 * To reduce code size, don't even check the stored silicon revision.
 */
#define AR_SREV_MERLIN(_ah) 0
#define AR_SREV_MERLIN_20(_ah) 0
#endif

#define AR_SREV_MERLIN_10_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version >= AR_SREV_VERSION_MERLIN))
#define AR_SREV_MERLIN_20_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version > AR_SREV_VERSION_MERLIN) ||   \
     ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_MERLIN) && \
      (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_MERLIN_20)))

#if defined(AH_SUPPORT_KIWI_ANY)
/*
 * The code is compiled to support Kiwi.
 * Check the stored silicon rev to see if the HW is a Kiwi chip.
 */
#define AR_SREV_KIWI(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI))

#if defined(AH_SUPPORT_KIWI_10) /* Kiwi 1.0 */
    /*
     * The code is compiled to support prototype revisions of Kiwi.
     * Check the stored silicon rev to see if the HW is a Kiwi prototype.
     */
    #define AR_SREV_KIWI_10(_ah) \
        ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI) && \
         (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_KIWI_10))
#else
    /* The code is not compiled to support Kiwi prototypes. */
    #define AR_SREV_KIWI_10(_ah) 0
#endif /* AH_SUPPORT_KIWI_10 */

#define AR_SREV_KIWI_11(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI) && \
     (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_KIWI_11))
#define AR_SREV_KIWI_12(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI) && \
     (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_KIWI_12))
#else
/*
 * The code is not compiled to support Kiwi.
 * To reduce code size, don't even check the stored silicon revision.
 */
#define AR_SREV_KIWI(_ah) 0
#define AR_SREV_KIWI_10(_ah) 0
#define AR_SREV_KIWI_11(_ah) 0
#define AR_SREV_KIWI_12(_ah) 0
#endif

#define AR_SREV_KIWI_10_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version >= AR_SREV_VERSION_KIWI))
#define AR_SREV_KIWI_11_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version > AR_SREV_VERSION_KIWI) ||   \
     ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI) && \
      (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_KIWI_11)))
#define AR_SREV_KIWI_12_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version > AR_SREV_VERSION_KIWI) ||   \
     ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI) && \
      (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_KIWI_12)))
#define AR_SREV_KIWI_13_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version > AR_SREV_VERSION_KIWI) || \
     ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KIWI) && \
      (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_KIWI_13)))

#if defined(AH_SUPPORT_KITE_ANY)
/*
 * The code is compiled to support Kite.
 * Check the stored silicon rev to see if the HW is a Kite chip.
 */
#define AR_SREV_KITE(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KITE))

#if defined(AH_SUPPORT_KITE_10) /* Kite 1.0 and 1.1 */
    /*
     * The code is compiled to support prototype revisions of Kite.
     * Check the stored silicon rev to see if the HW is a Kite prototype.
     */
    #define AR_SREV_KITE_11(_ah) \
      (AR_SREV_KITE(_ah) && \
       (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_KITE_11))
#else
    /* The code is not compiled to support Kite prototypes. */
    #define AR_SREV_KITE_11(_ah) 0
#endif /* AH_SUPPORT_KITE_10 */

#define AR_SREV_KITE_12(_ah) \
    (AR_SREV_KITE(_ah) && \
     (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_KITE_12))
#else
/*
 * The code is not compiled to support Kite.
 * To reduce code size, don't even check the stored silicon revision.
 */
#define AR_SREV_KITE(_ah) 0
#define AR_SREV_KITE_11(_ah) 0
#define AR_SREV_KITE_12(_ah) 0
#endif

#define AR_SREV_KITE_10_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version >= AR_SREV_VERSION_KITE))
#define AR_SREV_KITE_11_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version > AR_SREV_VERSION_KITE) || \
     ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KITE) && \
      (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_KITE_11)))
#define AR_SREV_KITE_12_OR_LATER(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version > AR_SREV_VERSION_KITE) || \
     ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_KITE) && \
      (AH_PRIVATE((_ah))->ah_mac_rev >= AR_SREV_REVISION_KITE_12)))
#define AR_SREV_ELIJAH_20(_ah) (AR_SREV_KITE_12_OR_LATER(_ah) && ((OS_REG_READ(_ah, AR_AN_SYNTH9) & 0x7) == 0x1))

#if defined(AH_SUPPORT_K2)
/*
 * The code is compiled to support K2.
 * Check the stored silicon rev to see if the HW is a K2 chip.
 */
#define AR_SREV_K2(_ah) \
    ((AH_PRIVATE((_ah))->ah_mac_version == AR_SREV_VERSION_K2))
#define AR_SREV_K2_10(_ah) \
    (AR_SREV_K2(_ah) && \
     (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_K2_10))
#define AR_SREV_K2_11(_ah) \
    (AR_SREV_K2(_ah) && \
     (AH_PRIVATE((_ah))->ah_mac_rev == AR_SREV_REVISION_K2_11))
#else
/*
 * The code is not compiled to support K2.
 * To reduce code size, don't even check the stored silicon revision.
 */
#define AR_SREV_K2(_ah) 0
#define AR_SREV_K2_10(_ah) 0
#define AR_SREV_K2_11(_ah) 0
#endif /* AH_SUPPORT_K2 */

#define AR_RADIO_SREV_MAJOR                   0xf0
#define AR_RAD5133_SREV_MAJOR                 0xc0 /* Fowl: 2+5G/3x3 */
#define AR_RAD2133_SREV_MAJOR                 0xd0 /* Fowl: 2G/3x3   */
#define AR_RAD5122_SREV_MAJOR                 0xe0 /* Fowl: 5G/2x2   */
#define AR_RAD2122_SREV_MAJOR                 0xf0 /* Fowl: 2+5G/2x2 */

#define AR_AHB_MODE                           0x4024 // ahb mode for dma
#define AR_AHB_EXACT_WR_EN                    0x00000000 // write exact bytes
#define AR_AHB_BUF_WR_EN                      0x00000001 // buffer write upto cacheline
#define AR_AHB_EXACT_RD_EN                    0x00000000 // read exact bytes
#define AR_AHB_CACHELINE_RD_EN                0x00000002 // read upto end of cacheline
#define AR_AHB_PREFETCH_RD_EN                 0x00000004 // prefetch upto page boundary
#define AR_AHB_PAGE_SIZE_1K                   0x00000000 // set page-size as 1k
#define AR_AHB_PAGE_SIZE_2K                   0x00000008 // set page-size as 2k
#define AR_AHB_PAGE_SIZE_4K                   0x00000010 // set page-size as 4k
#define AR_AHB_CUSTOM_BURST_EN                0x000000C0 // set Custom Burst Mode
#define AR_AHB_CUSTOM_BURST_EN_S              6 // set Custom Burst Mode
#define AR_AHB_CUSTOM_BURST_ASYNC_FIFO_VAL    3 // set both bits in Async FIFO mode


#define AR_INTR_RTC_IRQ                       0x00000001 // rtc in shutdown state
#define AR_INTR_MAC_IRQ                       0x00000002 // pending mac interrupt
#define AR_INTR_EEP_PROT_ACCESS               0x00000004 // eeprom protected area access
#define AR_INTR_MAC_AWAKE                     0x00020000 // mac is awake
#define AR_INTR_MAC_ASLEEP                    0x00040000 // mac is asleep
#define AR_INTR_SPURIOUS                      0xFFFFFFFF
/* TODO: fill in other values */

#define AR_INTR_SYNC_CAUSE_CLR                0x4028     // clear interrupt

#define AR_INTR_SYNC_CAUSE                    0x4028     // check pending interrupts

#define AR_INTR_SYNC_ENABLE                   0x402c     // enable interrupts
#define AR_INTR_SYNC_ENABLE_GPIO              0xFFFC0000 // enable interrupts: bits 18..31
#define AR_INTR_SYNC_ENABLE_GPIO_S            18         // enable interrupts: bits 18..31

/*
 * synchronous interrupt signals
 */
enum {
    AR_INTR_SYNC_RTC_IRQ                = 0x00000001,
    AR_INTR_SYNC_MAC_IRQ                = 0x00000002,
    AR_INTR_SYNC_EEPROM_ILLEGAL_ACCESS  = 0x00000004,
    AR_INTR_SYNC_APB_TIMEOUT            = 0x00000008,
    AR_INTR_SYNC_PCI_MODE_CONFLICT      = 0x00000010,
    AR_INTR_SYNC_HOST1_FATAL            = 0x00000020,
    AR_INTR_SYNC_HOST1_PERR             = 0x00000040,
    AR_INTR_SYNC_TRCV_FIFO_PERR         = 0x00000080,
    AR_INTR_SYNC_RADM_CPL_EP            = 0x00000100,
    AR_INTR_SYNC_RADM_CPL_DLLP_ABORT    = 0x00000200,
    AR_INTR_SYNC_RADM_CPL_TLP_ABORT     = 0x00000400,
    AR_INTR_SYNC_RADM_CPL_ECRC_ERR      = 0x00000800,
    AR_INTR_SYNC_RADM_CPL_TIMEOUT       = 0x00001000,
    AR_INTR_SYNC_LOCAL_TIMEOUT          = 0x00002000,
    AR_INTR_SYNC_PM_ACCESS              = 0x00004000,
    AR_INTR_SYNC_MAC_AWAKE              = 0x00008000,
    AR_INTR_SYNC_MAC_ASLEEP             = 0x00010000,
    AR_INTR_SYNC_MAC_SLEEP_ACCESS       = 0x00020000,
    AR_INTR_SYNC_ALL                    = 0x0003FFFF,

    /*
     * Do not enable and turn on mask for both sync and async interrupt, since
     * chip can generate interrupt storm.
     */
    AR_INTR_SYNC_DEFAULT                = (AR_INTR_SYNC_HOST1_FATAL         | 
                                           AR_INTR_SYNC_HOST1_PERR          |
                                           AR_INTR_SYNC_RADM_CPL_EP         |
                                           AR_INTR_SYNC_RADM_CPL_DLLP_ABORT |
                                           AR_INTR_SYNC_RADM_CPL_TLP_ABORT  |
                                           AR_INTR_SYNC_RADM_CPL_ECRC_ERR   |
                                           AR_INTR_SYNC_RADM_CPL_TIMEOUT    |
                                           AR_INTR_SYNC_LOCAL_TIMEOUT       |
                                           AR_INTR_SYNC_MAC_SLEEP_ACCESS),

    AR_INTR_SYNC_SPURIOUS               = 0xFFFFFFFF,

};

#define AR_INTR_ASYNC_MASK                       0x4030     // asynchronous interrupt mask
#define AR_INTR_ASYNC_MASK_GPIO                  0xFFFC0000 // asynchronous interrupt mask: bits 18..31
#define AR_INTR_ASYNC_MASK_GPIO_S                18         // asynchronous interrupt mask: bits 18..31

#define AR_INTR_SYNC_MASK                        0x4034     // synchronous interrupt mask
#define AR_INTR_SYNC_MASK_GPIO                   0xFFFC0000 // synchronous interrupt mask: bits 18..31
#define AR_INTR_SYNC_MASK_GPIO_S                 18         // synchronous interrupt mask: bits 18..31

#define AR_INTR_ASYNC_CAUSE_CLR                  0x4038     // check pending interrupts
#define AR_INTR_ASYNC_CAUSE                      0x4038     // check pending interrupts
#define AR_INTR_ASYNC_CAUSE_GPIO                 0xFFFC0000 // GPIO interrupts: bits 18..31
#define AR_INTR_ASYNC_USED                       (AR_INTR_MAC_IRQ | AR_INTR_ASYNC_CAUSE_GPIO)

#define AR_INTR_ASYNC_ENABLE                     0x403c     // enable interrupts
#define AR_INTR_ASYNC_ENABLE_GPIO                0xFFFC0000 // enable interrupts: bits 18..31
#define AR_INTR_ASYNC_ENABLE_GPIO_S              18         // enable interrupts: bits 18..31

/* PCIe defines */
#define AR_PCIE_SERDES                           0x4040
#define AR_PCIE_SERDES2                          0x4044
#define AR_PCIE_PM_CTRL                          0x4014
#define AR_PCIE_PM_CTRL_ENA                      0x00080000

#define AR_NUM_GPIO                              14 // Fourteen numbered 0 to 13.
#define AR928X_NUM_GPIO                          10 // Ten numbered 0 to 9 for Merlin.
#define AR9285_NUM_GPIO                          12 // Twelve numbered 0 to 11 for Kite.
#define AR9287_NUM_GPIO                          11 // Eleven numbered 0 to 10 for Kiwi.
#define AR9271_NUM_GPIO                          16 // Sixteen numbered 0 to 15 for K2.
#define AR7010_NUM_GPIO                          16 // Sixteen numbered 0 to 15 for Magpie.

#define AR_GPIO_IN_OUT                           0x4048 // GPIO input / output register
#define AR_GPIO_IN_VAL                           0x0FFFC000
#define AR_GPIO_IN_VAL_S                         14
#define AR928X_GPIO_IN_VAL                       0x000FFC00
#define AR928X_GPIO_IN_VAL_S                     10
#define AR9285_GPIO_IN_VAL                       0x00FFF000
#define AR9285_GPIO_IN_VAL_S                     12
#define AR9271_GPIO_IN_VAL                       0xFFFF0000
#define AR9271_GPIO_IN_VAL_S                     16
#define AR9287_GPIO_IN_VAL                       0x003FF800
#define AR9287_GPIO_IN_VAL_S                     11
#define AR7010_GPIO_IN_VAL                       0x0000FFFF
#define AR7010_GPIO_IN_VAL_S                     0


#define AR_GPIO_OE_OUT                           0x404c // GPIO output register
#define AR_GPIO_OE_OUT_DRV                       0x3    // 2 bit field mask, shifted by 2*bitpos
#define AR_GPIO_OE_OUT_DRV_NO                    0x0    // tristate
#define AR_GPIO_OE_OUT_DRV_LOW                   0x1    // drive if low
#define AR_GPIO_OE_OUT_DRV_HI                    0x2    // drive if high
#define AR_GPIO_OE_OUT_DRV_ALL                   0x3    // drive always

#define AR_GPIO_INTR_POL                         0x4050        // GPIO interrup polarity: 0 == high level, 1 == lo level
#define AR_GPIO_INTR_POL_VAL                     0x00001FFF    // bits 13:0 correspond to gpio 13:0
#define AR_GPIO_INTR_POL_VAL_S                   0             // bits 13:0 correspond to gpio 13:0

#define AR_GPIO_INPUT_EN_VAL                     0x4054     // GPIO input enable and value
#define AR_GPIO_INPUT_EN_VAL_BT_PRIORITY_DEF     0x00000004 // default value for bt_priority_async
#define AR_GPIO_INPUT_EN_VAL_BT_PRIORITY_S       2
#define AR_GPIO_INPUT_EN_VAL_BT_FREQUENCY_DEF    0x00000008 // default value for bt_frequency_async
#define AR_GPIO_INPUT_EN_VAL_BT_FREQUENCY_S      3
#define AR_GPIO_INPUT_EN_VAL_BT_ACTIVE_DEF       0x00000010 // default value for bt_active_async
#define AR_GPIO_INPUT_EN_VAL_BT_ACTIVE_S         4
#define AR_GPIO_INPUT_EN_VAL_RFSILENT_DEF        0x00000080 // default value for rfsilent_bb_l
#define AR_GPIO_INPUT_EN_VAL_RFSILENT_DEF_S      7
#define AR_GPIO_INPUT_EN_VAL_BT_PRIORITY_BB      0x00000400 // 0 == set bt_priority_async to default, 1 == connect bt_prority_async to baseband
#define AR_GPIO_INPUT_EN_VAL_BT_PRIORITY_BB_S    10
#define AR_GPIO_INPUT_EN_VAL_BT_FREQUENCY_BB     0x00000800 // 0 == set bt_frequency_async to default, 1 == connect bt_frequency_async to baseband
#define AR_GPIO_INPUT_EN_VAL_BT_FREQUENCY_BB_S   11
#define AR_GPIO_INPUT_EN_VAL_BT_ACTIVE_BB        0x00001000 // 0 == set bt_active_async to default, 1 == connect bt_active_async to baseband
#define AR_GPIO_INPUT_EN_VAL_BT_ACTIVE_BB_S      12
#define AR_GPIO_INPUT_EN_VAL_RFSILENT_BB         0x00008000 // 0 == set rfsilent_bb_l to default, 1 == connect rfsilent_bb_l to baseband
#define AR_GPIO_INPUT_EN_VAL_RFSILENT_BB_S       15
#define AR_GPIO_RTC_RESET_OVERRIDE_ENABLE        0x00010000
#define AR_GPIO_JTAG_DISABLE                     0x00020000 // 1 == disable JTAG

#define AR_GPIO_INPUT_MUX1                       0x4058
#define AR_GPIO_INPUT_MUX1_BT_PRIORITY           0x00000f00 /* bits 8..11: input mux for BT priority input */
#define AR_GPIO_INPUT_MUX1_BT_PRIORITY_S         8          /* bits 8..11: input mux for BT priority input */
#define AR_GPIO_INPUT_MUX1_BT_FREQUENCY          0x0000f000 /* bits 12..15: input mux for BT frequency input */
#define AR_GPIO_INPUT_MUX1_BT_FREQUENCY_S        12         /* bits 12..15: input mux for BT frequency input */
#define AR_GPIO_INPUT_MUX1_BT_ACTIVE             0x000f0000 /* bits 16..19: input mux for BT active input */
#define AR_GPIO_INPUT_MUX1_BT_ACTIVE_S           16         /* bits 16..19: input mux for BT active input */

#define AR_GPIO_INPUT_MUX2                       0x405c
#define AR_GPIO_INPUT_MUX2_CLK25                 0x0000000f // bits 0..3: input mux for clk25 input
#define AR_GPIO_INPUT_MUX2_CLK25_S               0          // bits 0..3: input mux for clk25 input
#define AR_GPIO_INPUT_MUX2_RFSILENT              0x000000f0 // bits 4..7: input mux for rfsilent_bb_l input
#define AR_GPIO_INPUT_MUX2_RFSILENT_S            4          // bits 4..7: input mux for rfsilent_bb_l input
#define AR_GPIO_INPUT_MUX2_RTC_RESET             0x00000f00 // bits 8..11: input mux for RTC Reset input
#define AR_GPIO_INPUT_MUX2_RTC_RESET_S           8          // bits 8..11: input mux for RTC Reset input

#define AR_GPIO_OUTPUT_MUX1                      0x4060
#define AR_GPIO_OUTPUT_MUX2                      0x4064
#define AR_GPIO_OUTPUT_MUX3                      0x4068

#define AR_GPIO_OUTPUT_MUX_AS_OUTPUT             0
#define AR_GPIO_OUTPUT_MUX_AS_PCIE_ATTENTION_LED 1
#define AR_GPIO_OUTPUT_MUX_AS_PCIE_POWER_LED     2
#define AR_GPIO_OUTPUT_MUX_AS_TX_FRAME           3
#define AR_GPIO_OUTPUT_MUX_AS_RX_CLEAR_EXTERNAL  4
#define AR_GPIO_OUTPUT_MUX_AS_MAC_NETWORK_LED    5
#define AR_GPIO_OUTPUT_MUX_AS_MAC_POWER_LED      6

#define AR_INPUT_STATE                           0x406c

#define AR7010_GPIO_OE                           0x52000 // Per bit output enable. 0-Output; 1-Input
#define AR7010_GPIO_OE_MASK                      0x1     // 1 bit field mask, shifted by 1*bitpos
#define AR7010_GPIO_OE_AS_OUTPUT                 0x0     // Config as output
#define AR7010_GPIO_OE_AS_INPUT                  0x1     // Config as input
#define AR7010_GPIO_IN                           0x52004 // GPIO current value
#define AR7010_GPIO_OUT                          0x52008 // Driver output value
#define AR7010_GPIO_SET                          0x5200C // Set
#define AR7010_GPIO_CLEAR                        0x52010 // Clear
#define AR7010_GPIO_INT                          0x52014 // Interrupt enable
#define AR7010_GPIO_INT_TYPE                     0x52018 // For each bit, 1-level-sensitive interrupt; 0-Edge-sensitive
#define AR7010_GPIO_INT_POLARITY                 0x5201C // For each bit, 1-activeHigh; 0-activeLow
#define AR7010_GPIO_PENDING                      0x52020 // For each bit, pending current interrupt
#define AR7010_GPIO_INT_MASK                     0x52024 // For each bit, interrupt mask
#define AR7010_GPIO_FUNCTION                     0x52028 // 

#define AR_EEPROM_STATUS_DATA                    0x407c
#define AR_EEPROM_STATUS_DATA_VAL                0x0000ffff
#define AR_EEPROM_STATUS_DATA_VAL_S              0
#define AR_EEPROM_STATUS_DATA_BUSY               0x00010000
#define AR_EEPROM_STATUS_DATA_BUSY_ACCESS        0x00020000
#define AR_EEPROM_STATUS_DATA_PROT_ACCESS        0x00040000
#define AR_EEPROM_STATUS_DATA_ABSENT_ACCESS      0x00080000

#define AR_OBS                                   0x4080
#define AR_GPIO_PDPU                             0x4088

#define AR_PCIE_MSI                              0x4094
#define AR_PCIE_MSI_ENABLE                       0x00000001

/* Sowl */
#define AR_RTC_SOWL_PLL_DIV    0x000003ff
#define AR_RTC_SOWL_PLL_DIV_S   0
#define AR_RTC_SOWL_PLL_REFDIV  0x00003C00
#define AR_RTC_SOWL_PLL_REFDIV_S 10
#define AR_RTC_SOWL_PLL_CLKSEL    0x0000C000
#define AR_RTC_SOWL_PLL_CLKSEL_S 14
// RTC registers
#ifndef AR9100
#define AR_RTC_RC               0x7000     /* reset control */
#define AR_RTC_RC_M             0x00000003
#define AR_RTC_RC_MAC_WARM      0x00000001
#define AR_RTC_RC_MAC_COLD      0x00000002

#define AR_RTC_PLL_CONTROL      0x7014
/* Owl */
#define AR_RTC_PLL_DIV          0x0000001f
#define AR_RTC_PLL_DIV_S        0
#define AR_RTC_PLL_DIV2         0x00000020
#define AR_RTC_PLL_REFDIV_5     0x000000c0
#define AR_RTC_PLL_CLKSEL       0x00000300
#define AR_RTC_PLL_CLKSEL_S     8

#define AR_RTC_RESET            0x7040      /* reset RTC */
#define AR_RTC_RESET_EN         0x00000001  /* Reset RTC bit */

#define AR_RTC_STATUS           0x7044      /* system sleep status */
#define AR_RTC_STATUS_M         0x0000000f
#define AR_RTC_STATUS_SHUTDOWN  0x00000001
#define AR_RTC_STATUS_ON        0x00000002
#define AR_RTC_STATUS_SLEEP     0x00000004
#define AR_RTC_STATUS_WAKEUP    0x00000008

#define AR_RTC_SLEEP_CLK            0x7048
#define AR_RTC_FORCE_DERIVED_CLK    0x2

#define AR_RTC_FORCE_WAKE           0x704c      /* control MAC force wake */
#define AR_RTC_FORCE_WAKE_EN        0x00000001  /* enable force wake */
#define AR_RTC_FORCE_WAKE_ON_INT    0x00000002  /* auto-wake on MAC interrupt */

#define AR_RTC_INTR_CAUSE       0x7050          /* RTC interrupt cause/clear */
#define AR_RTC_INTR_ENABLE      0x7054          /* RTC interrupt enable */
#define AR_RTC_INTR_MASK        0x7058          /* RTC interrupt mask */
#else
#define    AR_SEQ_MASK                0x8060    /* MAC AES mute mask */

#define AR_RTC_BASE             0x00020000
#define AR_RTC_RC               (AR_RTC_BASE + 0x0000)     /* reset control */
#define AR_RTC_RC_M             0x00000003
#define AR_RTC_RC_MAC_WARM      0x00000001
#define AR_RTC_RC_MAC_COLD      0x00000002
#define AR_RTC_RC_COLD_RESET    0x00000004
#define AR_RTC_RC_WARM_RESET    0x00000008

#define AR_RTC_PLL_CONTROL      (AR_RTC_BASE + 0x0014)
#define AR_RTC_PLL_DIV          0x0000001f
#define AR_RTC_PLL_DIV_S        0
#define AR_RTC_PLL_DIV2         0x00000020
#define AR_RTC_PLL_REFDIV_5     0x000000c0
#define AR_RTC_PLL_CLKSEL_S     8
#define AR_RTC_PLL_CLKSEL       0x00000300

#define AR_RTC_DERIVED_CLK      (AR_RTC_BASE + 0x0038)  /* RTC derived clock */
#define AR_RTC_DERIVED_CLK_PERIOD 0x0000fffe
#define AR_RTC_DERIVED_CLK_PERIOD_S 1

#define AR_RTC_RESET            (AR_RTC_BASE + 0x0040)      /* reset RTC */
#define AR_RTC_RESET_EN         0x00000001  /* Reset RTC bit */

#define AR_RTC_STATUS           (AR_RTC_BASE + 0x0044)      /* system sleep status */
#define AR_RTC_PM_STATUS_M      0x0000000f  /* Pwr Mgmt Status is the last 4 bits */
#define AR_RTC_STATUS_M         0x0000003f  /* RTC Status is the last 6 bits */
#define AR_RTC_STATUS_SHUTDOWN  0x00000001
#define AR_RTC_STATUS_ON        0x00000002
#define AR_RTC_STATUS_SLEEP     0x00000004
#define AR_RTC_STATUS_WAKEUP    0x00000008

#define AR_RTC_SLEEP_CLK            (AR_RTC_BASE + 0x0048)
#define AR_RTC_FORCE_DERIVED_CLK    0x2

#define AR_RTC_FORCE_WAKE           (AR_RTC_BASE + 0x004c)      /* control MAC force wake */
#define AR_RTC_FORCE_WAKE_EN        0x00000001  /* enable force wake */
#define AR_RTC_FORCE_WAKE_ON_INT    0x00000002  /* auto-wake on MAC interrupt */

#define AR_RTC_INTR_CAUSE       (AR_RTC_BASE + 0x0050)          /* RTC interrupt cause/clear */
#define AR_RTC_INTR_ENABLE      (AR_RTC_BASE + 0x0054)          /* RTC interrupt enable */
#define AR_RTC_INTR_MASK        (AR_RTC_BASE + 0x0058)          /* RTC interrupt mask */

/* Howl MAC revision that does not support Multiple BSSID Aggregation (Howl 1.1,1.2,1.3)*/
#define ATH_SREV_REV_HOWL_NO_MBSSID_AGGR   0xb0  
/* Howl MAC revision MASK to find the Howl 1.1,1.2,1.3 */
#define ATH_SREV_REV_HOWL_MASK             0xF8 
#endif //HOWL

// AR9280: rf long shift registers
#define AR_AN_RF2G1_CH0         0x7810
#define AR_AN_RF2G1_CH0_OB      0x03800000
#define AR_AN_RF2G1_CH0_OB_S    23
#define AR_AN_RF2G1_CH0_DB      0x1C000000
#define AR_AN_RF2G1_CH0_DB_S    26

#define AR_AN_RF5G1_CH0         0x7818
#define AR_AN_RF5G1_CH0_OB5     0x00070000
#define AR_AN_RF5G1_CH0_OB5_S   16
#define AR_AN_RF5G1_CH0_DB5     0x00380000
#define AR_AN_RF5G1_CH0_DB5_S   19

#define AR_AN_RF2G1_CH1         0x7834
#define AR_AN_RF2G1_CH1_OB      0x03800000
#define AR_AN_RF2G1_CH1_OB_S    23
#define AR_AN_RF2G1_CH1_DB      0x1C000000
#define AR_AN_RF2G1_CH1_DB_S    26

#define AR_AN_RF5G1_CH1         0x783C
#define AR_AN_RF5G1_CH1_OB5     0x00070000
#define AR_AN_RF5G1_CH1_OB5_S   16
#define AR_AN_RF5G1_CH1_DB5     0x00380000
#define AR_AN_RF5G1_CH1_DB5_S   19

#define AR_AN_TOP1              0x7890
#define AR_AN_TOP1_DACIPMODE    0x00040000
#define AR_AN_TOP1_DACIPMODE_S  18

#define AR_AN_TOP2                  0x7894
#define AR_AN_TOP2_XPABIAS_LVL      0xC0000000
#define AR_AN_TOP2_XPABIAS_LVL_S    30
#define AR_AN_TOP2_LOCALBIAS        0x00200000
#define AR_AN_TOP2_LOCALBIAS_S      21
#define AR_AN_TOP2_PWDCLKIND        0x00400000
#define AR_AN_TOP2_PWDCLKIND_S      22

#define AR_AN_SYNTH9            0x7868
#define AR_AN_SYNTH9_REFDIVA    0xf8000000
#define AR_AN_SYNTH9_REFDIVA_S  27

// AR9285 Analog registers 
#define AR9285_AN_RF2G1              0x7820
#define AR9285_AN_RF2G1_ENPACAL      0x00000800
#define AR9285_AN_RF2G1_ENPACAL_S    11
#define AR9285_AN_RF2G1_PDPADRV1     0x02000000
#define AR9285_AN_RF2G1_PDPADRV1_S   25
#define AR9285_AN_RF2G1_PDPADRV2     0x01000000
#define AR9285_AN_RF2G1_PDPADRV2_S   24
#define AR9285_AN_RF2G1_PDPAOUT      0x00800000
#define AR9285_AN_RF2G1_PDPAOUT_S    23


#define AR9285_AN_RF2G2              0x7824
#define AR9285_AN_RF2G2_OFFCAL       0x00001000
#define AR9285_AN_RF2G2_OFFCAL_S     12

#define AR9285_AN_RF2G3             0x7828
#define AR9285_AN_RF2G3_PDVCCOMP    0x02000000
#define AR9285_AN_RF2G3_PDVCCOMP_S  25
#define AR9285_AN_RF2G3_OB_0    0x00E00000
#define AR9285_AN_RF2G3_OB_0_S    21
#define AR9285_AN_RF2G3_OB_1    0x001C0000
#define AR9285_AN_RF2G3_OB_1_S    18
#define AR9285_AN_RF2G3_OB_2    0x00038000
#define AR9285_AN_RF2G3_OB_2_S    15
#define AR9285_AN_RF2G3_OB_3    0x00007000
#define AR9285_AN_RF2G3_OB_3_S    12
#define AR9285_AN_RF2G3_OB_4    0x00000E00
#define AR9285_AN_RF2G3_OB_4_S    9

#define AR9285_AN_RF2G3_DB1_0    0x000001C0
#define AR9285_AN_RF2G3_DB1_0_S    6
#define AR9285_AN_RF2G3_DB1_1    0x00000038
#define AR9285_AN_RF2G3_DB1_1_S    3
#define AR9285_AN_RF2G3_DB1_2    0x00000007
#define AR9285_AN_RF2G3_DB1_2_S    0
#define AR9285_AN_RF2G4         0x782C
#define AR9285_AN_RF2G4_DB1_3    0xE0000000
#define AR9285_AN_RF2G4_DB1_3_S    29
#define AR9285_AN_RF2G4_DB1_4    0x1C000000
#define AR9285_AN_RF2G4_DB1_4_S    26

#define AR9285_AN_RF2G4_DB2_0    0x03800000
#define AR9285_AN_RF2G4_DB2_0_S    23
#define AR9285_AN_RF2G4_DB2_1    0x00700000
#define AR9285_AN_RF2G4_DB2_1_S    20
#define AR9285_AN_RF2G4_DB2_2    0x000E0000
#define AR9285_AN_RF2G4_DB2_2_S    17
#define AR9285_AN_RF2G4_DB2_3    0x0001C000
#define AR9285_AN_RF2G4_DB2_3_S    14
#define AR9285_AN_RF2G4_DB2_4    0x00003800
#define AR9285_AN_RF2G4_DB2_4_S    11

#define AR9285_RF2G5                         0x7830
#define AR9285_RF2G5_IC50TX                  0xfffff8ff
#define AR9285_RF2G5_IC50TX_SET              0x00000400
#define AR9285_RF2G5_IC50TX_ELIJAH_SET       0x00000500
#define AR9285_RF2G5_IC50TX_CLEAR            0x00000700
#define AR9285_RF2G5_IC50TX_CLEAR_S          8

/* K2 : 0x7828, 0x782c different setting from Kite */                
#define AR9271_AN_RF2G3_OB_cck    0x001C0000          /* ob_0 */
#define AR9271_AN_RF2G3_OB_cck_S    18
#define AR9271_AN_RF2G3_OB_psk    0x00038000          /* ob_1 */
#define AR9271_AN_RF2G3_OB_psk_S    15
#define AR9271_AN_RF2G3_OB_qam    0x00007000          /* ob_2 */
#define AR9271_AN_RF2G3_OB_qam_S    12

#define AR9271_AN_RF2G3_DB_1    0x00E00000            /* db1_0 */
#define AR9271_AN_RF2G3_DB_1_S    21

#define AR9271_AN_RF2G3_CCOMP   0xFFF
#define AR9271_AN_RF2G3_CCOMP_S   0

#define AR9271_AN_RF2G4_DB_2    0xE0000000            /* db2_0 */
#define AR9271_AN_RF2G4_DB_2_S    29


#define AR9285_AN_RF2G6                 0x7834
#define AR9285_AN_RF2G6_CCOMP           0x00007800
#define AR9285_AN_RF2G6_CCOMP_S         11
#define AR9285_AN_RF2G6_OFFS            0x03f00000
#define AR9285_AN_RF2G6_OFFS_S          20

#define AR9271_AN_RF2G6_OFFS            0x07f00000
#define AR9271_AN_RF2G6_OFFS_S            20


#define AR9285_AN_RF2G7                 0x7838
#define AR9285_AN_RF2G7_PWDDB           0x00000002
#define AR9285_AN_RF2G7_PWDDB_S         1
#define AR9285_AN_RF2G7_PADRVGN2TAB0    0xE0000000
#define AR9285_AN_RF2G7_PADRVGN2TAB0_S  29

#define AR9285_AN_RF2G8                  0x783C
#define AR9285_AN_RF2G8_PADRVGN2TAB0     0x0001C000
#define AR9285_AN_RF2G8_PADRVGN2TAB0_S   14


#define AR9285_AN_RF2G9          0x7840
#define AR9285_AN_RXTXBB1              0x7854
#define AR9285_AN_RXTXBB1_PDRXTXBB1    0x00000020
#define AR9285_AN_RXTXBB1_PDRXTXBB1_S  5
#define AR9285_AN_RXTXBB1_PDV2I        0x00000080
#define AR9285_AN_RXTXBB1_PDV2I_S      7
#define AR9285_AN_RXTXBB1_PDDACIF      0x00000100
#define AR9285_AN_RXTXBB1_PDDACIF_S    8
#define AR9285_AN_RXTXBB1_SPARE9       0x00000001
#define AR9285_AN_RXTXBB1_SPARE9_S    0 

#define AR9285_AN_TOP2           0x7868

#define AR9285_AN_TOP3                  0x786c
#define AR9285_AN_TOP3_XPABIAS_LVL      0x0000000C
#define AR9285_AN_TOP3_XPABIAS_LVL_S    2
#define AR9285_AN_TOP3_PWDDAC           0x00800000
#define AR9285_AN_TOP3_PWDDAC_S    23

#define AR9285_AN_TOP4           0x7870
#define AR9285_AN_TOP4_DEFAULT   0x10142c00


#define AR9287_AN_RF2G3_CH0             0x7808
#define AR9287_AN_RF2G3_CH1             0x785c
#define AR9287_AN_RF2G3_DB1             0xE0000000
#define AR9287_AN_RF2G3_DB1_S           29
#define AR9287_AN_RF2G3_DB2             0x1C000000
#define AR9287_AN_RF2G3_DB2_S           26
#define AR9287_AN_RF2G3_OB_CCK          0x03800000
#define AR9287_AN_RF2G3_OB_CCK_S        23
#define AR9287_AN_RF2G3_OB_PSK          0x00700000
#define AR9287_AN_RF2G3_OB_PSK_S        20
#define AR9287_AN_RF2G3_OB_QAM          0x000E0000
#define AR9287_AN_RF2G3_OB_QAM_S        17
#define AR9287_AN_RF2G3_OB_PAL_OFF      0x0001C000
#define AR9287_AN_RF2G3_OB_PAL_OFF_S    14

#define AR9287_AN_TXPC0                 0x7898
#define AR9287_AN_TXPC0_TXPCMODE        0x0000C000
#define AR9287_AN_TXPC0_TXPCMODE_S      14
#define AR9287_AN_TXPC0_TXPCMODE_NORMAL    0
#define AR9287_AN_TXPC0_TXPCMODE_TEST      1
#define AR9287_AN_TXPC0_TXPCMODE_TEMPSENSE 2
#define AR9287_AN_TXPC0_TXPCMODE_ATBTEST   3

#define AR9287_AN_TOP2                  0x78b4
#define AR9287_AN_TOP2_XPABIAS_LVL      0xC0000000
#define AR9287_AN_TOP2_XPABIAS_LVL_S    30


// MAC PCU Registers
#define AR_STA_ID0                 0x8000 // MAC station ID0 - low 32 bits
#define AR_STA_ID1                 0x8004 // MAC station ID1 - upper 16 bits
#define AR_STA_ID1_SADH_MASK       0x0000FFFF // Mask for 16 msb of MAC addr
#define AR_STA_ID1_STA_AP          0x00010000 // Device is AP
#define AR_STA_ID1_ADHOC           0x00020000 // Device is ad-hoc
#define AR_STA_ID1_PWR_SAV         0x00040000 // Power save in generated frames
#define AR_STA_ID1_KSRCHDIS        0x00080000 // Key search disable
#define AR_STA_ID1_PCF             0x00100000 // Observe PCF
#define AR_STA_ID1_USE_DEFANT      0x00200000 // Use default antenna
#define AR_STA_ID1_DEFANT_UPDATE   0x00400000 // Update default ant w/TX antenna
#define AR_STA_ID1_RTS_USE_DEF     0x00800000 // Use default antenna to send RTS
#define AR_STA_ID1_ACKCTS_6MB      0x01000000 // Use 6Mb/s rate for ACK & CTS
#define AR_STA_ID1_BASE_RATE_11B   0x02000000 // Use 11b base rate for ACK & CTS
#define AR_STA_ID1_SECTOR_SELF_GEN 0x04000000 // default ant for generated frames
#define AR_STA_ID1_CRPT_MIC_ENABLE 0x08000000 // Enable Michael
#define AR_STA_ID1_KSRCH_MODE      0x10000000 // Look-up unique key when !keyID
#define AR_STA_ID1_PRESERVE_SEQNUM 0x20000000 // Don't replace seq num
#define AR_STA_ID1_CBCIV_ENDIAN    0x40000000 // IV endian-ness in CBC nonce
#define AR_STA_ID1_MCAST_KSRCH     0x80000000 // Adhoc key search enable

#define AR_BSS_ID0          0x8008 // MAC BSSID low 32 bits
#define AR_BSS_ID1          0x800C // MAC BSSID upper 16 bits / AID
#define AR_BSS_ID1_U16       0x0000FFFF // Mask for upper 16 bits of BSSID
#define AR_BSS_ID1_AID       0x07FF0000 // Mask for association ID
#define AR_BSS_ID1_AID_S     16         // Shift for association ID

#define AR_BCN_RSSI_AVE      0x8010 // MAC Beacon average RSSI
#define AR_BCN_RSSI_AVE_MASK 0x00000FFF // Beacon RSSI mask

#define AR_TIME_OUT         0x8014 // MAC ACK & CTS time-out
#define AR_TIME_OUT_ACK      0x00003FFF // Mask for ACK time-out
#define AR_TIME_OUT_ACK_S    0
#define AR_TIME_OUT_CTS      0x3FFF0000 // Mask for CTS time-out
#define AR_TIME_OUT_CTS_S    16
#define AR_TIME_OUT_ACK_CTS_ASYNC_FIFO_DUR    0x16001D56

#define AR_RSSI_THR          0x8018 // beacon RSSI warning / bmiss threshold
#define AR_RSSI_THR_MASK     0x000000FF // Beacon RSSI warning threshold
#define AR_RSSI_THR_BM_THR   0x0000FF00 // Mask for Missed beacon threshold
#define AR_RSSI_THR_BM_THR_S 8          // Shift for Missed beacon threshold
#define AR_RSSI_BCN_WEIGHT   0x1F000000 // RSSI average weight
#define AR_RSSI_BCN_WEIGHT_S 24
#define AR_RSSI_BCN_RSSI_RST 0x20000000 // Reset RSSI value

#define AR_USEC              0x801c // MAC transmit latency register
#define AR_USEC_USEC         0x0000007F // Mask for clock cycles in 1 usec
#define AR_USEC_TX_LAT       0x007FC000 // tx latency to start of SIGNAL (usec)
#define AR_USEC_TX_LAT_S     14         // tx latency to start of SIGNAL (usec)
#define AR_USEC_RX_LAT       0x1F800000 // rx latency to start of SIGNAL (usec)
#define AR_USEC_RX_LAT_S     23         // rx latency to start of SIGNAL (usec)
#define AR_USEC_ASYNC_FIFO_DUR    0x12e00074  //transmit latency value in Async FIFO mode

#define AR_RESET_TSF        0x8020
#define AR_RESET_TSF_ONCE   0x01000000 // reset tsf once ; self-clears bit
#define AR_RESET_TSF2_ONCE  0x02000000 // reset tsf2 once. For Kiwi only

#define AR_BEACON                       0x8020  /* MAC beacon control value/mode bits */
#define AR_CFP_PERIOD                   0x8024  /* MAC CFP Interval (TU/msec) */
#define AR_TIMER0                       0x8028  /* MAC Next beacon time (TU/msec) */
#define AR_TIMER1                       0x802c  /* MAC DMA beacon alert time (1/8 TU) */
#define AR_TIMER2                       0x8030  /* MAC Software beacon alert (1/8 TU) */
#define AR_TIMER3                       0x8034  /* MAC ATIM window time */
#define AR_MAX_CFP_DUR      0x8038 // MAC maximum CFP duration
#define AR_CFP_VAL          0x0000FFFF // CFP value in uS

#define AR_RX_FILTER        0x803C // MAC receive filter register
#define AR_RX_FILTER_ALL    0x00000000 // Disallow all frames
#define AR_RX_UCAST         0x00000001 // Allow unicast frames
#define AR_RX_MCAST         0x00000002 // Allow multicast frames
#define AR_RX_BCAST         0x00000004 // Allow broadcast frames
#define AR_RX_CONTROL       0x00000008 // Allow control frames
#define AR_RX_BEACON        0x00000010 // Allow beacon frames
#define AR_RX_PROM          0x00000020 // Promiscuous mode all packets
#define AR_RX_PROBE_REQ     0x00000080 // Any probe request frameA
#define AR_RX_MY_BEACON     0x00000200 // Any beacon frame with matching BSSID
#define AR_RX_COMPR_BAR     0x00000400 // Compressed directed block ack request
#define AR_RX_COMPR_BA      0x00000800 // Compressed directed block ack
#define AR_RX_UNCOM_BA_BAR  0x00001000 // Uncompressed directed BA or BAR

#define AR_PHY_ERR_MASK_REG    0x8338

#if 0
#define AR_RX_XR_POLL       0x00000040 // Allow XR Poll frames
#endif

#define AR_MCAST_FIL0       0x8040 // MAC multicast filter lower 32 bits
#define AR_MCAST_FIL1       0x8044 // MAC multicast filter upper 32 bits

#define AR_DIAG_SW                  0x8048 // MAC PCU control register
#define AR_DIAG_CACHE_ACK           0x00000001 // disable ACK when no valid key
#define AR_DIAG_ACK_DIS             0x00000002 // disable ACK generation
#define AR_DIAG_CTS_DIS             0x00000004 // disable CTS generation
#define AR_DIAG_ENCRYPT_DIS         0x00000008 // disable encryption
#define AR_DIAG_DECRYPT_DIS         0x00000010 // disable decryption
#define AR_DIAG_RX_DIS              0x00000020 // disable receive
#define AR_DIAG_LOOP_BACK           0x00000040 // enable loopback
#define AR_DIAG_CORR_FCS            0x00000080 // corrupt FCS
#define AR_DIAG_CHAN_INFO           0x00000100 // dump channel info
#if 0
#define AR_DIAG_EN_SCRAMSD   0x00000200 // enable fixed scrambler seed
#endif
#define AR_DIAG_SCRAM_SEED          0x0001FE00 // Mask for fixed scrambler seed
#define AR_DIAG_SCRAM_SEED_S        8        // Shift for fixed scrambler seed
#define AR_DIAG_FRAME_NV0           0x00020000 // accept w/protocol version !0
#define AR_DIAG_OBS_PT_SEL1         0x000C0000 // observation point select
#define AR_DIAG_OBS_PT_SEL1_S       18 // Shift for observation point select
#define AR_DIAG_FORCE_RX_CLEAR      0x00100000 // force rx_clear high
#define AR_DIAG_IGNORE_VIRT_CS      0x00200000 // ignore virtual carrier sense
#define AR_DIAG_FORCE_CH_IDLE_HIGH  0x00400000 // force channel idle high
#define AR_DIAG_EIFS_CTRL_ENA       0x00800000 // use framed and ~wait_wep if 0
#define AR_DIAG_DUAL_CHAIN_INFO     0x01000000 // dual chain channel info
#define AR_DIAG_RX_ABORT            0x02000000 //  abort rx
#define AR_DIAG_SATURATE_CYCLE_CNT  0x04000000 // saturate cycle cnts (no shift)
#define AR_DIAG_OBS_PT_SEL2         0x08000000 // Mask for observation point sel
#define AR_DIAG_RX_CLEAR_CTL_LOW    0x10000000 // force rx_clear (ctl) low (i.e. busy)
#define AR_DIAG_RX_CLEAR_EXT_LOW    0x20000000 // force rx_clear (ext) low (i.e. busy)

#define AR_TSF_L32          0x804c // MAC local clock lower 32 bits
#define AR_TSF_U32          0x8050 // MAC local clock upper 32 bits

#define AR_TST_ADDAC                 0x8054 // ADDAC test register
#define AR_TST_ADDAC_TST_MODE        0x1
#define AR_TST_ADDAC_TST_MODE_S      1
#define AR_TST_ADDAC_TST_LOOP_ENA    0x2
#define AR_TST_ADDAC_TST_LOOP_ENA_S  2
#define AR_TST_ADDAC_BEGIN_CAPTURE   0x80000
#define AR_TST_ADDAC_BEGIN_CAPTURE_S 19

#define AR_DEF_ANTENNA      0x8058 // default antenna register
#if 0
#define AR_DEF_ANT_CHN_SEL  0x4    // Default Ant Chain Select bit
#define AR_DEF_ANT_CHN0_ANT 0x1    // Def Ant Chain 0 Antenna Select bit
#define AR_DEF_ANT_CHN1_ANT 0x2    // Def Ant Chain 1 Antenna Select bit
#endif

#define AR_AES_MUTE_MASK0       0x805c // MAC AES mute mask
#define AR_AES_MUTE_MASK0_FC    0x0000FFFF // frame ctrl mask bits
#define AR_AES_MUTE_MASK0_QOS   0xFFFF0000 // qos ctrl mask bits
#define AR_AES_MUTE_MASK0_QOS_S 16

#define AR_AES_MUTE_MASK1       0x8060 // MAC AES mute mask
#define AR_AES_MUTE_MASK1_SEQ   0x0000FFFF // seq + frag mask bits

#define AR_GATED_CLKS       0x8064     // control clock domain
#define AR_GATED_CLKS_TX    0x00000002
#define AR_GATED_CLKS_RX    0x00000004
#define AR_GATED_CLKS_REG   0x00000008

#define AR_OBS_BUS_CTRL     0x8068  // select a bus for observation
#define AR_OBS_BUS_SEL_1    0x00040000
#define AR_OBS_BUS_SEL_2    0x00080000
#define AR_OBS_BUS_SEL_3    0x000C0000
#define AR_OBS_BUS_SEL_4    0x08040000
#define AR_OBS_BUS_SEL_5    0x08080000

#define AR_OBS_BUS_1               0x806c // mac debug observation bus
#define AR_OBS_BUS_1_PCU           0x00000001
#define AR_OBS_BUS_1_RX_END        0x00000002
#define AR_OBS_BUS_1_RX_WEP        0x00000004
#define AR_OBS_BUS_1_RX_BEACON     0x00000008
#define AR_OBS_BUS_1_RX_FILTER     0x00000010
#define AR_OBS_BUS_1_TX_HCF        0x00000020
#define AR_OBS_BUS_1_QUIET_TIME    0x00000040
#define AR_OBS_BUS_1_CHAN_IDLE     0x00000080
#define AR_OBS_BUS_1_TX_HOLD       0x00000100
#define AR_OBS_BUS_1_TX_FRAME      0x00000200
#define AR_OBS_BUS_1_RX_FRAME      0x00000400
#define AR_OBS_BUS_1_RX_CLEAR      0x00000800
#define AR_OBS_BUS_1_WEP_STATE     0x0003F000
#define AR_OBS_BUS_1_WEP_STATE_S   12
#define AR_OBS_BUS_1_RX_STATE      0x01F00000
#define AR_OBS_BUS_1_RX_STATE_S    20
#define AR_OBS_BUS_1_TX_STATE      0x7E000000
#define AR_OBS_BUS_1_TX_STATE_S    25

#define AR_OBSERV_2         0x8068  /* Observation bus 2 */
#define AR_OBSERV_1         0x806c  /* Observation bus 1 */
#define AR_PCU_SMPS                  0x8070         // Dynamic SM power save register
#define AR_PCU_SMPS_MAC_CHAINMASK    0x00000001     // Use the Rx Chainmask of MAC's setting
#define AR_PCU_SMPS_HW_CTRL_EN       0x00000002     // Enable hardware control of dynamic MIMO PS 
#define AR_PCU_SMPS_SW_CTRL_HPWR     0x00000004     // Software controlled High power chainmask setting
#define AR_PCU_SMPS_LPWR_CHNMSK      0x00000070     // Low power setting of Rx Chainmask
#define AR_PCU_SMPS_LPWR_CHNMSK_S    4
#define AR_PCU_SMPS_HPWR_CHNMSK      0x00000700     // High power setting of Rx Chainmask
#define AR_PCU_SMPS_HPWR_CHNMSK_S    8
#define AR_PCU_SMPS_LPWR_CHNMSK_VAL  0x1

#define AR_LAST_TSTP        0x8080 // MAC Time stamp of the last beacon received
#define AR_NAV              0x8084 // MAC current NAV value
#define AR_RTS_OK           0x8088 // MAC RTS exchange success counter
#define AR_RTS_FAIL         0x808c // MAC RTS exchange failure counter
#define AR_ACK_FAIL         0x8090 // MAC ACK failure counter
#define AR_FCS_FAIL         0x8094 // FCS check failure counter
#define AR_BEACON_CNT       0x8098 // Valid beacon counter

#define AR_XRMODE                   0x80c0     // Extended range mode
#define AR_XRSTMP                   0x80d0     // Extended range stomp

#if 0
#define AR_XRMODE_XR_POLL_TYPE_M    0x00000003 // poll type mask
#define AR_XRMODE_XR_POLL_TYPE_S    0
#define AR_XRMODE_XR_POLL_SUBTYPE_M 0x0000003c // poll type mask
#define AR_XRMODE_XR_POLL_SUBTYPE_S 2
#define AR_XRMODE_XR_WAIT_FOR_POLL  0x00000080 // wait for poll sta only
#define AR_XRMODE_XR_FRAME_HOLD_M   0xfff00000 // cycles hold for chirps
#define AR_XRMODE_XR_FRAME_HOLD_S   20

#define AR_XRDEL                    0x80c4     // Extended range delay
#define AR_XRDEL_SLOT_DELAY_M       0x0000ffff // cycles
#define AR_XRDEL_SLOT_DELAY_S       0
#define AR_XRDEL_CHIRP_DATA_DELAY_M 0xffff0000 // cycles
#define AR_XRDEL_CHIRP_DATA_DELAY_S 16

#define AR_XRTO                     0x80c8     // Extended range timeout
#define AR_XRTO_CHIRP_TO_M          0x0000ffff // cycles
#define AR_XRTO_CHIRP_TO_S          0
#define AR_XRTO_POLL_TO_M           0xffff0000 // cycles
#define AR_XRTO_POLL_TO_S           16

#define AR_XRCRP                    0x80cc     // Extended range chirp
#define AR_XRCRP_SEND_CHIRP         0x00000001 // generate stand alone chirp
#define AR_XRCRP_CHIRP_GAP_M        0xffff0000 // cycles
#define AR_XRCRP_CHIRP_GAP_S        16

#define AR_XRSTMP_RX_ABORT_RSSI     0x00000001 // stomp low rssi receive
#define AR_XRSTMP_RX_ABORT_BSSID    0x00000002 // stomp foreign bssid receive
#define AR_XRSTMP_TX_STOMP_RSSI     0x00000004 // xmit stomp low rssi receive
#define AR_XRSTMP_TX_STOMP_BSSID    0x00000008 // xmit stomp foreign bssid rx
#define AR_XRSTMP_TX_STOMP_DATA     0x00000010 // xmit stomp receive data
#define AR_XRSTMP_RX_ABORT_DATA     0x00000020 // stomp receive data
#define AR_XRSTMP_TX_RSSI_THRESH_M  0x0000FF00 // threshold for tx stomp
#define AR_XRSTMP_TX_RSSI_THRESH_S  8
#define AR_XRSTMP_RX_RSSI_THRESH_M  0x00FF0000 // threshold for tx stomp
#define AR_XRSTMP_RX_RSSI_THRESH_S  16
#endif

#define AR_SLEEP1               0x80d4           // Enhanced sleep control 1
#if 0
#define AR_SLEEP1_NEXT_DTIM_M       0x0007ffff // Absolute time(1/8TU) for next dtim mask
#define AR_SLEEP1_NEXT_DTIM_S       0          // Absolute time(1/8TU) for next dtim shift
#endif
#define AR_SLEEP1_ASSUME_DTIM   0x00080000 // Assume DTIM on missed beacon
#if 0
#define AR_SLEEP1_ENH_SLEEP_ENABLE  0x00100000 // Enables Venice sleep logic
#endif
#define AR_SLEEP1_CAB_TIMEOUT   0xFFE00000 // Cab timeout(TU) mask
#define AR_SLEEP1_CAB_TIMEOUT_S 21         // Cab timeout(TU) shift

#define AR_SLEEP2                   0x80d8     // Enhanced sleep control 2
#if 0
#define AR_SLEEP2_NEXT_TIM_M        0x0007ffff // Absolute time(1/8TU) for next tim/beacon mask
#define AR_SLEEP2_NEXT_TIM_S        0          // Absolute time(1/8TU) for next tim/beacon shift
#endif
#define AR_SLEEP2_BEACON_TIMEOUT    0xFFE00000 // Beacon timeout(TU) mask
#define AR_SLEEP2_BEACON_TIMEOUT_S  21         // Beacon timeout(TU) shift

#define AR_SLEEP3          0x80dc           // Enhanced sleep control 3
#if 0
#define AR_SLEEP3_TIM_PERIOD_M      0x0000ffff // Tim/Beacon period(TU) mask
#define AR_SLEEP3_TIM_PERIOD_S      0          // Tim/Beacon period(TU) shift
#define AR_SLEEP3_DTIM_PERIOD_M     0xffff0000 // DTIM period(TU) mask
#define AR_SLEEP3_DTIM_PERIOD_S     16         // DTIM period(TU) shift
#endif

#define AR_BSSMSKL            0x80e0           // BSSID mask lower 32 bits
#define AR_BSSMSKU            0x80e4           // BSSID mask upper 16 bits

#define AR_TPC                 0x80e8   // Transmit power control for gen frames
#define AR_TPC_ACK             0x0000003f // ack frames mask
#define AR_TPC_ACK_S           0x00       // ack frames shift
#define AR_TPC_CTS             0x00003f00 // cts frames mask
#define AR_TPC_CTS_S           0x08       // cts frames shift
#define AR_TPC_CHIRP           0x003f0000 // chirp frames mask
#define AR_TPC_CHIRP_S         0x16       // chirp frames shift

#define AR_TFCNT           0x80ec   // Profile count transmit frames
#define AR_RFCNT           0x80f0   // Profile count receive frames
#define AR_RCCNT           0x80f4   // Profile count receive clear
#define AR_CCCNT           0x80f8   // Profile count cycle counter

#define AR_QUIET1          0x80fc                   // Quiet time programming for TGh
#define AR_QUIET1_NEXT_QUIET_S         0            // TSF of next quiet period (TU)
#define AR_QUIET1_NEXT_QUIET_M         0x0000ffff
#define AR_QUIET1_QUIET_ENABLE         0x00010000   // Enable Quiet time operation
#define AR_QUIET1_QUIET_ACK_CTS_ENABLE 0x00020000   // ack/cts in quiet period
#define AR_QUIET1_QUIET_ACK_CTS_ENABLE_S 17
#define AR_QUIET2          0x8100                   // More Quiet programming
#define AR_QUIET2_QUIET_PERIOD_S       0            // Periodicity of quiet period (TU)
#define AR_QUIET2_QUIET_PERIOD_M       0x0000ffff
#define AR_QUIET2_QUIET_DUR_S     16           // quiet period (TU)
#define AR_QUIET2_QUIET_DUR       0xffff0000

#define AR_TSF_PARM        0x8104           // TSF parameters
#define AR_TSF_INCREMENT_M     0x000000ff
#define AR_TSF_INCREMENT_S     0x00

#define AR_QOS_NO_ACK              0x8108 // locate no_ack in qos
#define AR_QOS_NO_ACK_TWO_BIT      0x0000000f // 2 bit sentinel for no-ack
#define AR_QOS_NO_ACK_TWO_BIT_S    0
#define AR_QOS_NO_ACK_BIT_OFF      0x00000070 // offset for no-ack
#define AR_QOS_NO_ACK_BIT_OFF_S    4
#define AR_QOS_NO_ACK_BYTE_OFF     0x00000180 // from end of header
#define AR_QOS_NO_ACK_BYTE_OFF_S   7

#define AR_PHY_ERR         0x810c           // Phy errors to be filtered
    /* XXX validate! XXX */
#define AR_PHY_ERR_DCHIRP      0x00000008   // Bit  3 enables double chirp
#define AR_PHY_ERR_RADAR       0x00000020   // Bit  5 is Radar signal
#define AR_PHY_ERR_OFDM_TIMING 0x00020000   // Bit 17 is false detect for OFDM
#define AR_PHY_ERR_CCK_TIMING  0x02000000   // Bit 25 is false detect for CCK

#define AR_RXFIFO_CFG          0x8114
#define AR_RXFIFO_CFG_REG_RD_ENA_S 11
#define AR_RXFIFO_CFG_REG_RD_ENA   (0x1 << AR_RXFIFO_CFG_REG_RD_ENA_S)

    /* XXX sub-fields? XXX */
#define AR_MIC_QOS_CONTROL 0x8118
#define AR_MIC_QOS_SELECT  0x811c

#define AR_PCU_MISC                0x8120        // PCU Miscellaneous Mode
#define AR_PCU_FORCE_BSSID_MATCH   0x00000001    // force bssid to match
#define AR_PCU_MIC_NEW_LOC_ENA     0x00000004    // tx/rx mic key are together
#define AR_PCU_TX_ADD_TSF          0x00000008    // add tx_tsf + int_tsf
#define AR_PCU_CCK_SIFS_MODE       0x00000010    // assume 11b sifs programmed
#define AR_PCU_RX_ANT_UPDT         0x00000800    // KC_RX_ANT_UPDATE
#define AR_PCU_TXOP_TBTT_LIMIT_ENA 0x00001000    // enforce txop / tbtt
#define AR_PCU_MISS_BCN_IN_SLEEP   0x00004000    // count bmiss's when sleeping
#define AR_PCU_BUG_12306_FIX_ENA   0x00020000    // use rx_clear to count sifs
#define AR_PCU_FORCE_QUIET_COLL    0x00040000    // kill xmit for channel change
#define AR_PCU_BT_ANT_PREVENT_RX   0x00100000
#define AR_PCU_BT_ANT_PREVENT_RX_S 20
#define AR_PCU_TBTT_PROTECT        0x00200000    // no xmit upto tbtt + 20 uS
#define AR_PCU_CLEAR_VMF           0x01000000    // clear vmf mode (fast cc)
#define AR_PCU_CLEAR_BA_VALID      0x04000000    // clear ba state
#define AR_PCU_SEL_EVM             0x08000000    // select EVM data or PLCP header
#define AR_PCU_ALWAYS_PERFORM_KEYSEARCH 0x10000000 /* always perform key search */


#define AR_FILT_OFDM           0x8124
#define AR_FILT_OFDM_COUNT     0x00FFFFFF        // count of filtered ofdm

#define AR_FILT_CCK            0x8128
#define AR_FILT_CCK_COUNT      0x00FFFFFF        // count of filtered cck

#define AR_PHY_ERR_1           0x812c
#define AR_PHY_ERR_1_COUNT     0x00FFFFFF        // phy errs that pass mask_1
#define AR_PHY_ERR_MASK_1      0x8130            // mask for err_1_count

#define AR_PHY_ERR_2           0x8134
#define AR_PHY_ERR_2_COUNT     0x00FFFFFF        // phy errs that pass mask_2
#define AR_PHY_ERR_MASK_2      0x8138            // mask for err_2_count

#define AR_PHY_COUNTMAX        (3 << 22)         // Max counted before intr
#define AR_MIBCNT_INTRMASK     (3 << 22)         // Mask top 2 bits of counters

#define AR_TSFOOR_THRESHOLD     0x813c           // interrupt if rx_tsf-int_tsf
#define AR_TSFOOR_THRESHOLD_VAL 0x0000FFFF       // field width

#define AR_PHY_ERR_3           0x8168
#define AR_PHY_ERR_3_COUNT     0x00FFFFFF        // phy errs that pass mask_3
#define AR_PHY_ERR_MASK_3      0x816c            // mask for err_3_count

#define AR_BT_COEX_MODE            0x8170
#define AR_BT_TIME_EXTEND          0x000000ff
#define AR_BT_TIME_EXTEND_S        0
#define AR_BT_TXSTATE_EXTEND       0x00000100
#define AR_BT_TXSTATE_EXTEND_S     8
#define AR_BT_TX_FRAME_EXTEND      0x00000200
#define AR_BT_TX_FRAME_EXTEND_S    9
#define AR_BT_MODE                 0x00000c00
#define AR_BT_MODE_S               10
#define AR_BT_QUIET                0x00001000
#define AR_BT_QUIET_S              12
#define AR_BT_QCU_THRESH           0x0001e000
#define AR_BT_QCU_THRESH_S         13
#define AR_BT_RX_CLEAR_POLARITY    0x00020000
#define AR_BT_RX_CLEAR_POLARITY_S  17
#define AR_BT_PRIORITY_TIME        0x00fc0000
#define AR_BT_PRIORITY_TIME_S      18
#define AR_BT_FIRST_SLOT_TIME      0xff000000
#define AR_BT_FIRST_SLOT_TIME_S    24

#define AR_BT_COEX_WEIGHT          0x8174
#define AR_BT_BT_WGHT              0x0000ffff
#define AR_BT_BT_WGHT_S            0
#define AR_BT_WL_WGHT              0xffff0000
#define AR_BT_WL_WGHT_S            16

#define AR_BT_COEX_MODE2           0x817c
#define AR_BT_BCN_MISS_THRESH      0x000000ff
#define AR_BT_BCN_MISS_THRESH_S    0
#define AR_BT_BCN_MISS_CNT         0x0000ff00
#define AR_BT_BCN_MISS_CNT_S       8
#define AR_BT_HOLD_RX_CLEAR        0x00010000
#define AR_BT_HOLD_RX_CLEAR_S      16
#define AR_BT_DISABLE_BT_ANT       0x00100000
#define AR_BT_DISABLE_BT_ANT_S     20


#define AR_TXSIFS              0x81d0
#define AR_TXSIFS_TIME         0x000000FF        // uS in SIFS
#define AR_TXSIFS_TX_LATENCY   0x00000F00        // uS for transmission thru bb
#define AR_TXSIFS_TX_LATENCY_S 8
#define AR_TXSIFS_ACK_SHIFT    0x00007000        // chan width for ack
#define AR_TXSIFS_ACK_SHIFT_S  12

#define AR_TXOP_X          0x81ec                // txop for legacy non-qos
#define AR_TXOP_X_VAL      0x000000FF

    /* on-demand subfields */
#define AR_TXOP_0_3    0x81f0                    // txop for various tid's
#define AR_TXOP_4_7    0x81f4
#define AR_TXOP_8_11   0x81f8
#define AR_TXOP_12_15  0x81fc

#if 0
#define AR_KC_MASK         0x81c4        // MAC Key Cache Mask for words 0x10 0x14
                                                // 0 is write allow 1 is write blocked
#define AR_KC_MASK_TYPE_M          0x00000007   // MAC Key Cache Type Mask
#define AR_KC_MASK_LAST_TX_ANT     0x00000008   // MAC Key Cache Last Tx Ant Mask
#define AR_KC_MASK_ASYNC_MASK_M    0x000001f0   // MAC Key Cache Async Rate Offset Mask
#define AR_KC_MASK_UPDT_BF         0x00000200   // MAC Key Cache Update Bf coef Mask
#define AR_KC_MASK_RX_CHAIN0_ACK   0x00000400   // MAC Key Cache Ack Ant Ch 0 Mask
#define AR_KC_MASK_RX_CHAIN1_ACK   0x00000800   // MAC Key Cache Ack Ant Ch 1 Mask
#define AR_KC_MASK_TX_CHAIN0_SEL   0x00001000   // MAC Key Cache Tx Sel Ant Ch 0 Mask
#define AR_KC_MASK_TX_CHAIN1_SEL   0x00002000   // MAC Key Cache Tx Sel Ant Ch 1 Mask
#define AR_KC_MASK_CHAIN_SEL       0x00004000   // MAC Key Cache Chain Sel Mask
#define AR_KC_MASK_WORD_10         0x00010000   // MAC Key Cache Word 0x10 Mask
#endif

/* generic timers based on tsf - all uS */
#define AR_NUM_GEN_TIMERS_PRE_MERLIN        8
#define AR_NUM_GEN_TIMERS_POST_MERLIN       16
#define AR_GEN_TIMER_BANK_1_LEN             8
#define AR_FIRST_NDP_TIMER                  7
#define AR_FIRST_KIWI_NDP_TIMER             7

#define AR_NEXT_TBTT_TIMER                  0x8200
#define AR_NEXT_DMA_BEACON_ALERT            0x8204
#define AR_NEXT_SWBA                        0x8208
#define AR_NEXT_CFP                         0x8208
#define AR_NEXT_HCF                         0x820C
#define AR_NEXT_TIM                         0x8210
#define AR_NEXT_DTIM                        0x8214
#define AR_NEXT_QUIET_TIMER                 0x8218
#define AR_NEXT_NDP_TIMER                   0x821C      // Generic timer 7
#define AR_NEXT_NDP2_TIMER                  0x8180      // Generic timer 8-15

#define AR_BEACON_PERIOD                    0x8220
#define AR_DMA_BEACON_PERIOD                0x8224
#define AR_SWBA_PERIOD                      0x8228
#define AR_HCF_PERIOD                       0x822C
#define AR_TIM_PERIOD                       0x8230
#define AR_DTIM_PERIOD                      0x8234
#define AR_QUIET_PERIOD                     0x8238
#define AR_NDP_PERIOD                       0x823C      // Generic timer 7
#define AR_NDP2_PERIOD                      0x81A0      // Generic timer 8-15

#define AR_NDP2_TIMER_MODE                  0x81C0      // Generic timer 8-15

#define AR_BT_COEX_WEIGHT2                  0x81c4

#define AR_TIMER_MODE                       0x8240
#define AR_TBTT_TIMER_EN                    0x00000001
#define AR_DBA_TIMER_EN                     0x00000002
#define AR_SWBA_TIMER_EN                    0x00000004
#define AR_HCF_TIMER_EN                     0x00000008
#define AR_TIM_TIMER_EN                     0x00000010
#define AR_DTIM_TIMER_EN                    0x00000020
#define AR_QUIET_TIMER_EN                   0x00000040
#define AR_NDP_TIMER_EN                     0x00000080
#define AR_TIMER_OVERFLOW_INDEX             0x00000700
#define AR_TIMER_OVERFLOW_INDEX_S           8
#define AR_TIMER_THRESH                     0xFFFFF000
#define AR_TIMER_THRESH_S                   12

#define AR_SLP32_MODE                  0x8244
#define AR_SLP32_HALF_CLK_LATENCY      0x000FFFFF    // rising <-> falling edge
#define AR_SLP32_ENA                   0x00100000
#define AR_SLP32_TSF_WRITE_STATUS      0x00200000    // tsf update in progress

#define AR_SLP32_WAKE              0x8248
#define AR_SLP32_WAKE_XTL_TIME     0x0000FFFF    // time to wake crystal

#define AR_SLP32_INC               0x824c
#define AR_SLP32_TST_INC           0x000FFFFF

#define AR_SLP_CNT         0x8250    // 32kHz cycles for which mac is asleep
#define AR_SLP_CYCLE_CNT   0x8254    // absolute number of 32kHz cycles

#define AR_SLP_MIB_CTRL    0x8258
#define AR_SLP_MIB_CLEAR   0x00000001    // clear pending
#define AR_SLP_MIB_PENDING 0x00000002    // clear counters

/* Used by Kiwi Async FIFO */
#define AR_MAC_PCU_LOGIC_ANALYZER               0x8264
#define AR_MAC_PCU_LOGIC_ANALYZER_DISBUG20768   0x20000000

#if ATH_SUPPORT_WAPI
/* Used for WAPI deaggregation */
#define AR_MAC_PCU_LOGIC_WAPI_DEAGGR_ENABLE     0x20000000
#endif

#ifdef AR5416_EMULATION
// MAC trace buffer registers (emulation only)
#define AR_MAC_PCU_LOGIC_ANALYZER               0x8264
#define AR_MAC_PCU_LOGIC_ANALYZER_CTL           0x0000000F
#define AR_MAC_PCU_LOGIC_ANALYZER_HOLD          0x00000001
#define AR_MAC_PCU_LOGIC_ANALYZER_CLEAR         0x00000002
#define AR_MAC_PCU_LOGIC_ANALYZER_STATE         0x00000004
#define AR_MAC_PCU_LOGIC_ANALYZER_ENABLE        0x00000008
#define AR_MAC_PCU_LOGIC_ANALYZER_QCU_SEL       0x000000F0
#define AR_MAC_PCU_LOGIC_ANALYZER_QCU_SEL_S     4
#define AR_MAC_PCU_LOGIC_ANALYZER_INT_ADDR      0x0003FF00
#define AR_MAC_PCU_LOGIC_ANALYZER_INT_ADDR_S    8

#define AR_MAC_PCU_LOGIC_ANALYZER_DIAG_MODE    0xFFFC0000
#define AR_MAC_PCU_LOGIC_ANALYZER_DIAG_MODE_S   18
#define AR_MAC_PCU_LOGIC_ANALYZER_DISBUG20614   0x00040000
#define AR_MAC_PCU_LOGIC_ANALYZER_DISBUG20768   0x20000000
#define AR_MAC_PCU_LOGIC_ANALYZER_DISBUG20803   0x40000000

#define AR_MAC_PCU_LOGIC_ANALYZER_32L           0x8268
#define AR_MAC_PCU_LOGIC_ANALYZER_16U           0x826C

#define AR_MAC_PCU_TRACE_REG_START      0xE000
#define AR_MAC_PCU_TRACE_REG_END        0xFFFC
#define AR_MAC_PCU_TRACE_BUFFER_LENGTH (AR_MAC_PCU_TRACE_REG_END - AR_MAC_PCU_TRACE_REG_START + sizeof(uint32_t))
#endif  // AR5416_EMULATION

#define AR_2040_MODE                0x8318
#define AR_2040_JOINED_RX_CLEAR 0x00000001   // use ctl + ext rx_clear for cca

/* Additional cycle counter. See also AR_CCCNT */
#define AR_EXTRCCNT         0x8328   // extension channel rx clear count
                     // counts number of cycles rx_clear (ext) is low (i.e. busy)
                     // when the MAC is not actively transmitting/receiving

#define AR_SELFGEN_MASK         0x832c

#define AR_PCU_TXBUF_CTRL               0x8340
#define AR_PCU_TXBUF_CTRL_SIZE_MASK     0x7FF
#define AR_PCU_TXBUF_CTRL_USABLE_SIZE   0x700
#define AR_KITE_PCU_TXBUF_CTRL_USABLE_SIZE   0x380

#define AR_PCU_MISC_MODE2               0x8344      /* Register documented in mac_pcu_reg.reg (only hw guys can see) */
#define AR_PCU_MISC_MODE2_BUG_21532_ENABLE             0x00000001
#define AR_PCU_MISC_MODE2_MGMT_CRYPTO_ENABLE           0x00000002   /* Decrypt MGT frames using MFP method */
#define AR_PCU_MISC_MODE2_NO_CRYPTO_FOR_NON_DATA_PKT   0x00000004   /* Don't decrypt MGT frames at all */
#define AR_PCU_MISC_MODE2_RESERVED                     0x00000038
#define AR_PCU_MISC_MODE2_ADHOC_MCAST_KEYID_ENABLE     0x00000040 /* This bit enables the Multicast search based on both MAC Address and Key ID. 
                                                                   * If bit is 0, then Multicast search is based on MAC address only.
                                                                   * For Merlin and above only. */
#define AR_PCU_MISC_MODE2_CFP_IGNORE                   0x00000080
#define AR_PCU_MISC_MODE2_MGMT_QOS                     0x0000FF00 
#define AR_PCU_MISC_MODE2_MGMT_QOS_S                   8
#define AR_PCU_MISC_MODE2_ENABLE_LOAD_NAV_BEACON_DURATION 0x00010000
#define AR_PCU_MISC_MODE2_ENABLE_AGGWEP                0x00020000
#define AR_PCU_MISC_MODE2_HWWAR1                       0x00100000
#define AR_PCU_MISC_MODE2_HWWAR2                       0x02000000
#define AR_PCU_MISC_MODE2_RESERVED2                    0xFFFE0000
#if ATH_SUPPORT_WAPI
#define AR_PCU_MISC_MODE2_BC_MC_WAPI_MODE			   0x00040000
#endif

#define AR_MAC_PCU_ASYNC_FIFO_REG3                     0x8358
#define AR_MAC_PCU_ASYNC_FIFO_REG3_DATAPATH_SEL        0x00000400
#define AR_MAC_PCU_ASYNC_FIFO_REG3_SOFT_RESET          0x80000000

/* TSF2. For Kiwi only */
#define AR_TSF2_L32                                    0x8390
#define AR_TSF2_U32                                    0x8394

/* MAC Direct Connect Control. For Kiwi only */
#define AR_DIRECT_CONNECT                              0x83A0
#define AR_DC_AP_STA_EN                                0x00000001

#define AR_AES_MUTE_MASK0       0x805c // MAC AES mute mask
#define AR_AES_MUTE_MASK0_FC    0x0000FFFF // frame ctrl mask bits
#define AR_AES_MUTE_MASK0_QOS   0xFFFF0000 // qos ctrl mask bits
#define AR_AES_MUTE_MASK0_QOS_S 16

#define AR_AES_MUTE_MASK1       0x8060 // MAC AES mute mask
#define AR_AES_MUTE_MASK1_SEQ   0x0000FFFF // seq + frag mask bits
#define AR_AES_MUTE_MASK1_SEQ_S        0 
#define AR_AES_MUTE_MASK1_FC_MGMT      0xFFFF0000 // frame ctrl mask for mgmt frames (Sowl)
#define AR_AES_MUTE_MASK1_FC_MGMT_S    16 
#if 0
#define AR_FRM_TYPE_CAP_TBL  0x8500      // Frame Type Capabilities Table
#define AR_FRM_TYPE_CAP_SIZE   64          // Frame Type Cap. Table Size
#define AR_FTC_BF_RX_UPDT_NORM 0x00000001  // BFCOEF_RX_UPDATE_NORMAL
#define AR_FTC_BF_RX_UPDT_SELF 0x00000002  // BFCOEF_RX_UPDATE_SELF_GEN
#define AR_FTC_BF_TX_ENB_NORM  0x00000004  // BFCOEF_TX_ENABLE_NORMAL
#define AR_FTC_BF_TX_ENB_SELF  0x00000008  // BFCOEF_TX_ENABLE_SELF_GEN
#define AR_FTC_BF_TX_ENB_GEN   0x00000010  // BFCOEF_TX_ENABLE_GEN
#define AR_FTC_BF_TX_ENB_MCAST 0x00000020  // BFCOEF_TX_ENABLE_MCAST
#endif

    //  rate duration registers - used for Multi-rate retry.
#define AR_RATE_DURATION_0   0x8700  // 32 registers from 0x8700 to 0x87CC
#define AR_RATE_DURATION_31  0x87CC
#define AR_RATE_DURATION_32  0x8780 
#define AR_RATE_DURATION(_n)            (AR_RATE_DURATION_0 + ((_n)<<2))

/* AZIMUTH mode reg can be used for proxySTA */
#define AR_AZIMUTH_MODE                 0x8300
#define AR_AZIMUTH_KEY_SEARCH_AD1       0x00000002
#define AR_AZIMUTH_CTS_MATCH_TX_AD2     0x00000040
#define AR_AZIMUTH_BA_USES_AD1          0x00000080

#define AR_KEYTABLE_0           0x8800  /* MAC Key Cache */
#define AR_KEYTABLE(_n)         (AR_KEYTABLE_0 + ((_n)*32))
#define AR_KEY_CACHE_SIZE       128
#define AR_RSVD_KEYTABLE_ENTRIES 4
#define AR_KEY_TYPE             0x00000007 // MAC Key Type Mask
#define AR_KEYTABLE_TYPE_40     0x00000000  /* WEP 40 bit key */
#define AR_KEYTABLE_TYPE_104    0x00000001  /* WEP 104 bit key */
#define AR_KEYTABLE_TYPE_128    0x00000003  /* WEP 128 bit key */
#define AR_KEYTABLE_TYPE_TKIP   0x00000004  /* TKIP and Michael */
#define AR_KEYTABLE_TYPE_AES    0x00000005  /* AES/OCB 128 bit key */
#if ATH_SUPPORT_WAPI
#define AR_KEYTABLE_TYPE_WAPI   0x00000005  /* WAPI 128 bit key */
#endif
#define AR_KEYTABLE_TYPE_CCM    0x00000006  /* AES/CCM 128 bit key */
#define AR_KEYTABLE_TYPE_CLR    0x00000007  /* no encryption */
#define AR_KEYTABLE_ANT         0x00000008  /* previous transmit antenna */
#define AR_KEYTABLE_VALID       0x00008000  /* key and MAC address valid */
#define AR_KEYTABLE_KEY0(_n)    (AR_KEYTABLE(_n) + 0)   /* key bit 0-31 */
#define AR_KEYTABLE_KEY1(_n)    (AR_KEYTABLE(_n) + 4)   /* key bit 32-47 */
#define AR_KEYTABLE_KEY2(_n)    (AR_KEYTABLE(_n) + 8)   /* key bit 48-79 */
#define AR_KEYTABLE_KEY3(_n)    (AR_KEYTABLE(_n) + 12)  /* key bit 80-95 */
#define AR_KEYTABLE_KEY4(_n)    (AR_KEYTABLE(_n) + 16)  /* key bit 96-127 */
#define AR_KEYTABLE_TYPE(_n)    (AR_KEYTABLE(_n) + 20)  /* key type */
#define AR_KEYTABLE_MAC0(_n)    (AR_KEYTABLE(_n) + 24)  /* MAC address 1-32 */
#define AR_KEYTABLE_MAC1(_n)    (AR_KEYTABLE(_n) + 28)  /* MAC address 33-47 */
#define AR_KEYTABLE_DIR_ACK_BIT    0x00000010  /* Directed ACK bit */

#define AR_QOS_CONTROL                  0x8118  /* Control TKIP MIC for QoS */
#define AR_QOS_SELECT                   0x811c  /* MIC QoS select */
#define AR_MISC_MODE                    0x8120  /* PCU Misc. mode control */

#define AR_RC                           0x4000  /* Warm reset control register */
#define AR_SCR                          0x4004  /* Sleep control register */
#define AR_INTPEND                      0x4008  /* Interrupt Pending register */
#define AR_SFR                          0x400C  /* Sleep force register */
#define AR_PCICFG                       0x4010  /* PCI configuration register */
#define AR_GPIOCR                       0x4014  /* GPIO control register */
#define AR_GPIODO                       0x4018  /* GPIO data output access register */
#define AR_GPIODI                       0x401C  /* GPIO data input access register */
#define AR_TXEPOST                      0x4028  /* TXE write posting resgister */
#define AR_QSM                          0x402C  /* QCU sleep mask */

#if ATH_WOW

#define    AR_PMCTRL_AUX_PWR_DET       0x10000000  /* Puts Chip in L2 state */
#define    AR_PMCTRL_D3COLD_VAUX       0x00800000
#define    AR_PMCTRL_HOST_PME_EN       0x00400000    /* Send OOB WAKE_L on WoW event */
#define    AR_PMCTRL_WOW_PME_CLR       0x00200000    /* Clear WoW event */
#define    AR_PMCTRL_PWR_STATE_MASK    0x0F000000    /* Power State Mask */
#define    AR_PMCTRL_PWR_STATE_D1D3    0x0F000000    /* Activate D1 and D3 */
#define    AR_PMCTRL_PWR_STATE_D0      0x08000000    /* Activate D0 */
#define    AR_PMCTRL_PWR_PM_CTRL_ENA   0x00008000    /* Enable power management */

#define    AR_WOW_BEACON_TIMO_MAX      0xFFFFFFFF  /* Max. value for Beacon Timeout */

/*
 * MAC WoW Registers.
 */
#define    AR_WOW_PATTERN_REG          0x825C
#define    AR_WOW_COUNT_REG            0x8260
#define    AR_WOW_BCN_EN_REG           0x8270
#define    AR_WOW_BCN_TIMO_REG         0x8274
#define    AR_WOW_KEEP_ALIVE_TIMO_REG  0x8278
#define    AR_WOW_KEEP_ALIVE_REG       0x827C
#define    AR_WOW_US_SCALAR_REG        0x8284
#define    AR_WOW_KEEP_ALIVE_DELAY_REG 0x8288
#define    AR_WOW_PATTERN_MATCH_REG    0x828C
#define    AR_WOW_PATTERN_OFF1_REG     0x8290 /* Pattern bytes 0 -> 3 */
#define    AR_WOW_PATTERN_OFF2_REG     0x8294 /* Pattern bytes 4 -> 7 */
/* For Kite or Later version of chips */
#define AR_WOW_EXACT_REG            0x829C
#define AR_WOW_LENGTH1_REG          0x8360
#define AR_WOW_LENGTH2_REG          0x8364
/* Register to enable pattern match for less than 256 bytes packets */
#define AR_WOW_PATTERN_MATCH_LT_256B_REG    0x8368


/* AR_WOW_PATTERN_REG Values */
#define    AR_WOW_BACK_OFF_SHIFT(x)    ((x & 0xf) << 27)    /* in usecs */
#define    AR_WOW_MAC_INTR_EN          0x00040000
#define    AR_WOW_MAGIC_EN             0x00010000
#define    AR_WOW_PATTERN_EN(x)        ((x & 0xff) << 0)
#define    AR_WOW_PATTERN_FOUND_SHIFT  8
#define    AR_WOW_PATTERN_FOUND(x)     (x & (0xff << AR_WOW_PATTERN_FOUND_SHIFT))    
#define    AR_WOW_PATTERN_FOUND_MASK   ((0xff) << AR_WOW_PATTERN_FOUND_SHIFT)    
#define    AR_WOW_MAGIC_PAT_FOUND      0x00020000
#define    AR_WOW_MAC_INTR             0x00080000
#define    AR_WOW_KEEP_ALIVE_FAIL      0x00100000
#define    AR_WOW_BEACON_FAIL          0x00200000

#define    AR_WOW_STATUS(x) (x & (AR_WOW_PATTERN_FOUND_MASK | AR_WOW_MAGIC_PAT_FOUND | \
                               AR_WOW_KEEP_ALIVE_FAIL | AR_WOW_BEACON_FAIL))
#define AR_WOW_CLEAR_EVENTS(x)     (x & ~(AR_WOW_PATTERN_EN(0xff) | \
       AR_WOW_MAGIC_EN | AR_WOW_MAC_INTR_EN | AR_WOW_BEACON_FAIL | \
       AR_WOW_KEEP_ALIVE_FAIL))

/* AR_WOW_COUNT_REG Values */
#define    AR_WOW_AIFS_CNT(x)          ((x & 0xff) << 0)
#define    AR_WOW_SLOT_CNT(x)          ((x & 0xff) << 8)
#define    AR_WOW_KEEP_ALIVE_CNT(x)    ((x & 0xff) << 16)

/* AR_WOW_BCN_EN_REG */
#define    AR_WOW_BEACON_FAIL_EN       0x00000001

/* AR_WOW_BCN_TIMO_REG */
#define    AR_WOW_BEACON_TIMO          0x40000000  /* Valid if BCN_EN is set */

/* AR_WOW_KEEP_ALIVE_TIMO_REG */
#define    AR_WOW_KEEP_ALIVE_TIMO      0x00007A12
#define    AR_WOW_KEEP_ALIVE_NEVER     0xFFFFFFFF

/* AR_WOW_KEEP_ALIVE_REG */
#define    AR_WOW_KEEP_ALIVE_AUTO_DIS  0x00000001
#define    AR_WOW_KEEP_ALIVE_FAIL_DIS  0x00000002

/* AR_WOW_KEEP_ALIVE_DELAY_REG */
#define    AR_WOW_KEEP_ALIVE_DELAY     0x000003E8 // 1 msec

/*
 * Keep it long for Beacon workaround - ensures no false alarm
 */
#define AR_WOW_BMISSTHRESHOLD       0x20

/* AR_WOW_PATTERN_MATCH_REG */
#define    AR_WOW_PAT_END_OF_PKT(x)    ((x & 0xf) << 0)
#define    AR_WOW_PAT_OFF_MATCH(x)     ((x & 0xf) << 8)

/*
 * Default values for Wow Configuration for backoff, aifs, slot, keep-alive, etc.
 * to be programmed into various registers.
 */
#define    AR_WOW_PAT_BACKOFF          0x00000004    // AR_WOW_PATTERN_REG
#define    AR_WOW_CNT_AIFS_CNT         0x00000022    // AR_WOW_COUNT_REG
#define    AR_WOW_CNT_SLOT_CNT         0x00000009    // AR_WOW_COUNT_REG
    /*
     * Keepalive count applicable for Merlin 2.0 and above.
     */
#define    AR_WOW_CNT_KA_CNT           0x00000008    // AR_WOW_COUNT_REG

/* WoW - Transmit buffer for keep alive frames */
#define AR_WOW_TRANSMIT_BUFFER      0xE000 /* E000 - EFFC */

#define AR_WOW_KA_DESC_WORD2        0xE000
#define AR_WOW_KA_DESC_WORD3        0xE004
#define AR_WOW_KA_DESC_WORD4        0xE008
#define AR_WOW_KA_DESC_WORD5        0xE00C
#define AR_WOW_KA_DESC_WORD6        0xE010
#define AR_WOW_KA_DESC_WORD7        0xE014
#define AR_WOW_KA_DESC_WORD8        0xE018
#define AR_WOW_KA_DESC_WORD9        0xE01C
#define AR_WOW_KA_DESC_WORD10       0xE020
#define AR_WOW_KA_DESC_WORD11       0xE024
#define AR_WOW_KA_DESC_WORD12       0xE028
#define AR_WOW_KA_DESC_WORD13       0xE02C

#define AR_WOW_KA_DATA_WORD0        0xE030
#define AR_WOW_KA_DATA_WORD1        0xE034
#define AR_WOW_KA_DATA_WORD2        0xE038
#define AR_WOW_KA_DATA_WORD3        0xE03C
#define AR_WOW_KA_DATA_WORD4        0xE040
#define AR_WOW_KA_DATA_WORD5        0xE044

/* WoW Transmit Buffer for patterns */
#define AR_WOW_TB_PATTERN0          0xE100
#define AR_WOW_TB_PATTERN1          0xE200
#define AR_WOW_TB_PATTERN2          0xE300
#define AR_WOW_TB_PATTERN3          0xE400
#define AR_WOW_TB_PATTERN4          0xE500
#define AR_WOW_TB_PATTERN5          0xE600
#define AR_WOW_TB_PATTERN6          0xE700
#define AR_WOW_TB_PATTERN7          0xE800
#define AR_WOW_TB_MASK0             0xEC00
#define AR_WOW_TB_MASK1             0xEC20
#define AR_WOW_TB_MASK2             0xEC40
#define AR_WOW_TB_MASK3             0xEC60
#define AR_WOW_TB_MASK4             0xEC80
#define AR_WOW_TB_MASK5             0xECa0
#define AR_WOW_TB_MASK6             0xECC0
#define AR_WOW_TB_MASK7             0xECE0

/* Currently Pattern 0-7 are supported - so bit 0-7 are set */
#define AR_WOW_PATTERN_SUPPORTED    0xFF
#define AR_WOW_LENGTH_MAX           0xFF
#define AR_WOW_LENGTH1_SHIFT(_i)    ((0x3 - ((_i) & 0x3)) << 0x3)
#define AR_WOW_LENGTH1_MASK(_i)     (AR_WOW_LENGTH_MAX << AR_WOW_LENGTH1_SHIFT(_i))
#define AR_WOW_LENGTH2_SHIFT(_i)    ((0x7 - ((_i) & 0x7)) << 0x3)
#define AR_WOW_LENGTH2_MASK(_i)     (AR_WOW_LENGTH_MAX << AR_WOW_LENGTH2_SHIFT(_i))

#endif /* ATH_WOW */

/* K2 */
#define AR9271_CLOCK_CONTROL              0x50040
#define AR9271_CLOCK_SELECTION_22         0x0
#define AR9271_CLOCK_SELECTION_88         0x1
#define AR9271_CLOCK_SELECTION_44         0x2
#define AR9271_CLOCK_SELECTION_117        0x4
#define AR9271_CLOCK_SELECTION_OSC_40     0x6
#define AR9271_CLOCK_SELECTION_RTC        0x7
#define AR9271_SPI_SEL                    0x100
#define AR9271_UART_SEL                   0x200

#define AR9271_RESET_POWER_DOWN_CONTROL   0x50044
#define AR9271_RADIO_RF_RST               0x20
#define AR9271_GATE_MAC_CTL               0x4000
#define AR9271_MAIN_PLL_PWD_CTL           0x40000

#define AR9271_CLKMISC                    0x4090
#define AR9271_OSC_to_10M_EN              0x00000001

/*
* Mask used to construct AAD for CCMP-AES
* Cisco spec defined bits 0-3 as mask 
* IEEE802.11w defined as bit 4.
*/
#define AR_MFP_QOS_MASK_IEEE      0x10 
#define AR_MFP_QOS_MASK_CISCO     0xf

/*
* frame control field mask:
* 0 0 0 0 0 0 0 0
* | | | | | | | | _ Order            bit
* | | | | | | | _ _ Protected Frame  bit
* | | | | | | _ _ _ More data        bit
* | | | | | _ _ _ _ Power management bit
* | | | | _ _ _ _ _ Retry            bit
* | | | _ _ _ _ _ _ More fragments   bit
* | | _ _ _ _ _ _ _ FromDS           bit
* | _ _ _ _ _ _ _ _ ToDS             bit
*/
#define AR_AES_MUTE_MASK1_FC_MGMT_MFP 0xC7FF

#endif

