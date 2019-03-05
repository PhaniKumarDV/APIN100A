/*
 * Copyright (c) 2004 Atheros Communications, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _DEV_ATH_AHB_H_
#define _DEV_ATH_AHB_H_

#include <asm/io.h>
#include <asm/uaccess.h>
#include <if_bus.h>
#define AR531X_WLAN0_NUM       0
#define AR531X_WLAN1_NUM       1

#define REG_WRITE(_reg,_val)		*((volatile u_int32_t *)(_reg)) = (_val);
#define REG_READ(_reg)		*((volatile u_int32_t *)(_reg))

/*
 * AR9330 specific settings
 */
#define AR9330_IRQ_WLAN_INTRS  2
#define AR9330_WLAN            0xB8100000
#define AR9330_WLAN_LEN        0x1ffff

/*
 * 5315 specific registers 
 */

/* 
 * PCI-MAC Configuration registers 
 */
#define AR5315_PCI              0xB0100000      /* PCI MMR */
#define AR5315_PCI_MAC_RC              (AR5315_PCI + 0x4000) 
#define AR5315_PCI_MAC_SCR             (AR5315_PCI + 0x4004)
#define AR5315_PCI_MAC_INTPEND         (AR5315_PCI + 0x4008)
#define AR5315_PCI_MAC_SFR             (AR5315_PCI + 0x400C)
#define AR5315_PCI_MAC_PCICFG          (AR5315_PCI + 0x4010)
#define AR5315_PCI_MAC_SREV            (AR5315_PCI + 0x4020)

#define AR5315_PCI_MAC_RC_MAC          0x00000001
#define AR5315_PCI_MAC_RC_BB           0x00000002

#define AR5315_PCI_MAC_SCR_SLMODE_M    0x00030000
#define AR5315_PCI_MAC_SCR_SLMODE_S    16        
#define AR5315_PCI_MAC_SCR_SLM_FWAKE   0         
#define AR5315_PCI_MAC_SCR_SLM_FSLEEP  1         
#define AR5315_PCI_MAC_SCR_SLM_NORMAL  2         

#define AR5315_PCI_MAC_SFR_SLEEP       0x00000001

#define AR5315_PCI_MAC_PCICFG_SPWR_DN  0x00010000

#define AR5315_IRQ_WLAN0_INTRS 3
#define AR5315_WLAN0           0xb0000000

#define AR5315_ENDIAN_CTL      0xb100000c
#define AR5315_CONFIG_WLAN            0x00000002      /* WLAN byteswap */
#define AR5315_AHB_ARB_CTL     0xb1000008
#define AR5315_ARB_WLAN               0x00000002
#ifdef ATH_SUPPORT_WSC
#define AR5317_WSC_GPIO      	0x34
#else
#define AR5317_WSC_GPIO      	-1
#endif

/*
 * Revision Register - Initial value is 0x3010 (WMAC 3.0, AR531X 1.0).
 */
#define AR5315_SREV             0xb1000014

#define AR5315_REV_MAJ                     0x0080
#define AR5315_REV_MAJ_M                   0x00f0
#define AR5315_REV_MAJ_S                   4
#define AR5315_REV_MIN_M                     0x000f
#define AR5315_REV_MIN_S                   0
#define AR5315_REV_CHIP                    (REV_MAJ|REV_MIN)

#define AR5317_REV_MAJ                     0x0090

#define AR531X_IRQ_WLAN0_INTRS 2
#define AR531X_IRQ_WLAN1_INTRS 5
#define AR531X_WLAN0           0xb8000000
#define AR531X_WLAN1           0xb8500000
#define AR531X_WLANX_LEN       0x000ffffc

#define AR531X_ENABLE          0xbc003080
#define AR531X_ENABLE_WLAN1    0x8
#define AR531X_ENABLE_WLAN0    0x1
#define AR531X_RADIO_MASK_OFF  0xc8
#define AR531X_RADIO0_MASK     0x0003
#define AR531X_RADIO1_MASK     0x000c
#define AR531X_RADIO1_S        2

#define AR531X_APBBASE         0xbc000000
#define AR531X_RESETTMR	       (AR531X_APBBASE  + 0x3000)
#define AR531X_REV             (AR531X_RESETTMR + 0x0090) /* revision */
#define AR531X_REV_MAJ         0x00f0
#define AR531X_REV_MAJ_S       4
#define AR531X_REV_MIN         0x000f
#define AR531X_REV_MIN_S       0

#define AR531X_BD_MAGIC 0x35333131   /* "5311", for all 531x platforms */


extern const char * get_system_type(void);
extern int valid_wmac_num(u_int16_t);
extern int get_wmac_irq(u_int16_t);
extern unsigned long get_wmac_base(u_int16_t);
extern unsigned long get_wmac_mem_len(u_int16_t);

#define sysRegRead(phys)      (*(volatile u_int32_t *)phys)

#endif    /* _DEV_ATH_AHB_H_ */
