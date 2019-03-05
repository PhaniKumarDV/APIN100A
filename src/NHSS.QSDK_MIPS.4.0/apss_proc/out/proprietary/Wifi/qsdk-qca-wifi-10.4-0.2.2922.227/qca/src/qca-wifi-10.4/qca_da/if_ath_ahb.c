/*
 * Copyright (c) 2010, Atheros Communications Inc.
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
/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

#include <osdep.h>
#ifdef USE_PLATFORM_FRAMEWORK
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#endif

#include "if_athvar.h"
#include "ah_devid.h"
#include "if_ath_ahb.h"

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#ifdef AH_CAL_IN_FLASH_AHB
static u_int32_t    CalAddr[AH_CAL_RADIOS_AHB] = {AH_CAL_LOCATIONS_AHB};
#endif

#if defined(AH_CAL_IN_DRAM_AHB) && defined(HOST_OFFLOAD)
static u_int32_t    CalAddr_dram[AH_CAL_RADIOS_AHB] = {AH_CAL_LOCATIONS_DRAM_AHB};
#endif

struct ath_ahb_softc {
	struct ath_softc_net80211   aps_sc;
	struct _NIC_DEV             aps_osdev;
#ifdef CONFIG_PM
	u32			aps_pmstate[16];
#endif
};

static struct ath_ahb_softc *sclist[2] = {NULL, NULL};
static u_int8_t num_activesc=0;

extern int __ath_attach(u_int16_t devid, struct net_device *dev, HAL_BUS_CONTEXT *bus_context, osdev_t osdev);
extern int __ath_detach(struct net_device *dev);
extern unsigned int ahbskip;

#ifdef USE_PLATFORM_FRAMEWORK
static struct platform_device *spdev;
#endif /* USE_PLATFORM_FRAMEWORK */

#ifdef HOST_OFFLOAD
extern u_int32_t hif_rom_mode(void);
#endif

int ahb_enable_wmac(u_int16_t devid,u_int16_t wlanNum)
{
        return 0;
}

#ifdef USE_PLATFORM_FRAMEWORK
int
get_wmac_irq(u_int16_t wmac_num)
{
	int ret;
	struct resource *res;

	res = platform_get_resource(spdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		printk("no IRQ resource found\n");
		ret = -ENXIO;
		goto out;
	}

	ret = res->start;
out:
	return ret;
}

unsigned long
get_wmac_base(u_int16_t wmac_num)
{
	void __iomem *mem = NULL;
	struct resource *res;
	unsigned long ret;

	res = platform_get_resource(spdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk("no memory resource found\n");
		ret = -ENXIO;
		goto err_out;
	}

	mem = ioremap_nocache(res->start, resource_size(res));
	if (mem == NULL) {
		printk("ioremap failed\n");
		ret = -ENOMEM;
		goto err_out;
	}

	return (unsigned long) mem;
err_out:
	iounmap(mem);
	return ret;
}

unsigned long
get_wmac_mem_len(u_int16_t wmac_num)
{
	struct resource *res;
	unsigned long ret;

	res = platform_get_resource(spdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&spdev->dev, "no memory resource found\n");
		ret = -ENXIO;
		goto out;
	}
	ret = resource_size(res);
out:
	return ret;
}
#endif /* USE_PLATFORM_FRAMEWORK */

static int
init_ath_wmac(u_int16_t devid, u_int16_t wlanNum)
{
    const char *athname;
    struct net_device *dev;
    struct ath_ahb_softc *sc;
    HAL_BUS_CONTEXT bc;

#ifdef AH_CAL_IN_DRAM_AHB
#ifdef USE_PLATFORM_FRAMEWORK
    struct ath9k_platform_data *spdata;
#endif
#endif

    if (wlanNum != 0 || sclist[wlanNum] != NULL) {
        return -ENODEV;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
    dev = alloc_netdev(sizeof(struct ath_ahb_softc), "wifi%d", 0, ether_setup);
#else
    dev = alloc_netdev(sizeof(struct ath_ahb_softc), "wifi%d", ether_setup);
#endif

    if (dev == NULL) {
        printk(KERN_ERR "%s: no memory for device state\n", __func__);
        goto bad1;
    }
    sc = ath_netdev_priv(dev);
    sc->aps_osdev.netdev = dev;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,41)
    dev->owner = THIS_MODULE;
#elif LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,24)
    SET_MODULE_OWNER(dev);

#endif

    sclist[wlanNum] = sc;

    dev->irq = get_wmac_irq(wlanNum);
	dev->mem_start = get_wmac_base(wlanNum);
	dev->mem_end = dev->mem_start + get_wmac_mem_len(wlanNum);

    /*
     * Don't leave arp type as ARPHRD_ETHER as this is no eth device
     */
    dev->type = ARPHRD_IEEE80211;

    sc->aps_osdev.bdev = NULL;

#ifdef AH_CAL_IN_FLASH_AHB
    bc.bc_info.bc_tag = (void *) CalAddr[0];
    bc.bc_info.cal_in_flash = 1;
#else
    bc.bc_info.bc_tag = NULL;
    bc.bc_info.cal_in_flash = 0;
#endif

#ifdef AH_CAL_IN_DRAM_AHB
#ifdef USE_PLATFORM_FRAMEWORK
    spdata = (struct ath9k_platform_data *) spdev->dev.platform_data;
    bc.bc_info.bc_tag = (void *) spdata->eeprom_data;
    bc.bc_info.cal_in_flash = 0;
#endif
#ifdef HOST_OFFLOAD
    bc.bc_info.bc_tag = (void *) CalAddr_dram[0];
    if(!hif_rom_mode())
        bc.bc_info.cal_in_flash = 1;
    else
        bc.bc_info.cal_in_flash = 0;
#endif
#endif

    bc.bc_handle = (HAL_BUS_HANDLE)dev->mem_start;
    bc.bc_bustype = HAL_BUS_TYPE_AHB;

    if (__ath_attach(devid, dev, &bc, &sc->aps_osdev) != 0) {
        goto bad2;
    }

    athname = ath_hal_probe(ATHEROS_VENDOR_ID, devid);

    printk(KERN_INFO "%s: %s: mem=0x%lx, irq=%d\n",
        dev->name, athname ? athname : "Atheros ???",
        dev->mem_start, dev->irq);

    num_activesc++;

    return 0;
bad2:
    free_netdev(dev);
bad1:
    sclist[wlanNum]=NULL;
    return -ENODEV;
}

static int
exit_ath_wmac(u_int16_t wlanNum)
{
    struct ath_ahb_softc *sc = sclist[wlanNum];
    struct net_device *dev;

    if(sc == NULL) {
        return -ENODEV; /* XXX: correct return value? */
    }

    dev = sc->aps_osdev.netdev;
    if(dev == NULL) {
        printk("%s: No device\n", __func__);
        return -ENODEV;
    }
    __ath_detach(dev);
    free_netdev(dev);

    sclist[wlanNum]=NULL;
    return 0;
}

#if !defined(USE_PLATFORM_FRAMEWORK)
int init_ahb(void)
{
	u_int16_t devid = 0;
	const char *sysType;

	sysType = get_system_type(); /* Get the devid from BSP */

	if(strcmp(sysType, "Atheros AR9330 (Hornet)") == 0) {
		devid = AR9300_DEVID_AR9380_PCIE;
	} else if ((strcmp(sysType,"QCA955x emu") == 0) ||
                   (strcmp(sysType,"Atheros AR934x emu") == 0) ||
                   (strcmp(sysType,"QCA953x emu") == 0))  {
		devid = AR9300_DEVID_EMU_PCIE;
	} else if(strncmp(sysType,"Atheros AR934", strlen("Atheros AR934")) == 0) {
		devid = AR9300_DEVID_AR9340;
	} else if(strcmp(sysType,"QCA955x") == 0) {
		devid = AR9300_DEVID_AR955X;
	} else if(strcmp(sysType, "QCA953x") == 0) {
		devid = AR9300_DEVID_AR953X;
    } else if(strcmp(sysType, "QCA956x") == 0) {
		devid = AR9300_DEVID_AR956X;
	} else if(strcmp(sysType, "QCA956x emu") == 0) {
		devid = AR9300_DEVID_AR956X;
	} else {
        printk("Unknow sysType: %s\n", sysType);
		return -ENODEV;
	}

    return init_ath_wmac(devid, 0);
}
#endif


/*
 * Module glue.
 */
#include "version.h"

#if !defined(ATH_PCI) || !defined(USE_PLATFORM_FRAMEWORK)
static char *version = ATH_PCI_VERSION " (Atheros/multi-bss)";
static char *dev_info = "ath_da_ahb";
#endif
#include <linux/ethtool.h>

#if !defined(ATH_PCI)
/*
 * Typically, only one of ATH_PCI or ATH_AHB will be defined.
 * This file getting compiled implies ATH_AHB is defined.
 * However, in the 'dual' bus case, the foll. definition and
 * the one from pci will conflict. Hence, knock this off if
 * PCI is also defined (i.e. dual case).
 */
int
ath_ioctl_ethtool(struct ath_softc *sc, int cmd, void *addr)
{
	struct ethtool_drvinfo info;

	if (cmd != ETHTOOL_GDRVINFO)
		return -EOPNOTSUPP;
	memset(&info, 0, sizeof(info));
	info.cmd = cmd;
	strncpy(info.driver, dev_info, sizeof(info.driver)-1);
	strncpy(info.version, version, sizeof(info.version)-1);
	return _copy_to_user(addr, &info, sizeof(info)) ? -EFAULT : 0;
}
#endif /* ATH_PCI */

MODULE_AUTHOR("Atheros Communications, Inc.");
MODULE_DESCRIPTION("Support for Atheros 802.11 wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("Atheros WLAN cards");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

#if !defined(USE_PLATFORM_FRAMEWORK)

int __init
init_ath_ahb(void)
{
    printk(KERN_INFO "%s: %s\n", dev_info, version);

	if (init_ahb() != 0) {
		printk("ath_ahb: No devices found, driver not installed.\n");
		return (-ENODEV);
	}

#ifdef notyet
#if defined(CONFIG_SYSCTL) && !defined(AR9100)
    ath_sysctl_register();
#endif
#endif
    return (0);
}

void __exit
exit_ath_ahb(void)
{
#ifdef notyet
#if defined(CONFIG_SYSCTL) && !defined(AR9100)
    ath_sysctl_unregister();
#endif
#endif

    exit_ath_wmac(0);

    printk(KERN_INFO "%s: driver unloaded\n", dev_info);
}

#else
static const struct platform_device_id ath9k_platform_id_table[] = {
    {
		.name = "ath9k",
		.driver_data = AR5416_AR9100_DEVID,
	},
	{
		.name = "ar933x_wmac",
		.driver_data = AR9300_DEVID_AR9380_PCIE,
	},
	{
		.name = "ar934x_wmac",
		.driver_data = AR9300_DEVID_AR9340,
	},
	{
		.name = "qca955x_wmac",
		.driver_data = AR9300_DEVID_AR955X,
	},
    {
        .name = "qca956x_wmac",
        .driver_data = AR9300_DEVID_AR956X,
    },
    {
        .name = "qca953x_wmac",
        .driver_data = AR9300_DEVID_AR953X,
    },
    {},
};

static int ath_ahb_probe(struct platform_device *pdev)
{
	const struct platform_device_id *id;
	int ret;

    id = platform_get_device_id(pdev);

    /* Do not register Direct attach devices if ahbskip is 1 */
    if(ahbskip) {
        ret = -EINVAL;
        goto out;
    }

	spdev = pdev;

    /* Assuming platform_data will be populated in case of non OF devices */
        if (!pdev->dev.platform_data) {
            printk("no platform data specified\n");
            ret = -EINVAL;
            goto out;
        }

	ret = init_ath_wmac(id->driver_data, 0);
out:
	return ret;
}

static int ath_ahb_remove(struct platform_device *pdev)
{
	spdev = NULL;

	exit_ath_wmac(0);
	return 0;
}

static struct platform_driver ath_ahb_driver = {
	.probe      = ath_ahb_probe,
	.remove     = ath_ahb_remove,
	.driver		= {
		.name	= "ath9k_ahb",
		.owner	= THIS_MODULE,
	},
	.id_table    = ath9k_platform_id_table,
};

MODULE_DEVICE_TABLE(platform, ath9k_platform_id_table);

int init_ath_ahb(void)
{
	return platform_driver_register(&ath_ahb_driver);
}

void exit_ath_ahb(void)
{
	platform_driver_unregister(&ath_ahb_driver);
}
#endif

#if !defined(ATH_PCI)
module_init(init_ath_ahb);
module_exit(exit_ath_ahb);
#endif /* ATH_PCI */
