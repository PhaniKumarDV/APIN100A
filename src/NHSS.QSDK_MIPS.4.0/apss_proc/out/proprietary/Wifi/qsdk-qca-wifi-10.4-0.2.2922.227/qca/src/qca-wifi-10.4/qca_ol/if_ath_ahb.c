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
 * Copyright (c) 2013, 2016 Qualcomm Atheros, Inc.
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

#if (ATH_PERF_PWR_OFFLOAD != 0) && defined(HIF_PCI)
#include "ol_if_athvar.h"
#include "ath_pci.h"
#endif

#include "if_athvar.h"
#include "ah_devid.h"
#include "if_ath_ahb.h"

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

extern unsigned int ahbskip;

#ifdef USE_PLATFORM_FRAMEWORK
static struct platform_device *spdev;
#endif /* USE_PLATFORM_FRAMEWORK */

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
    return 0;
}

static int
exit_ath_wmac(u_int16_t wlanNum)
{
    return 0;
}

/*
 * Module glue.
 */
#include "version.h"

#if !defined(ATH_PCI) || !defined(USE_PLATFORM_FRAMEWORK)
static char *version = ATH_PCI_VERSION " (Atheros/multi-bss)";
static char *dev_info = "ath_ol_ahb";
#endif
#include <linux/ethtool.h>

MODULE_AUTHOR("Atheros Communications, Inc.");
MODULE_DESCRIPTION("Support for Atheros 802.11 wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("Atheros WLAN cards");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

#if !defined(USE_PLATFORM_FRAMEWORK)

#else
static const struct platform_device_id ath9k_platform_id_table[] = {
	{
		.name = "ipq40xx_wmac", /* Update the name with proper device name (ipqxxxx) */
		.driver_data = IPQ4019_DEVICE_ID,
    },
    {},
};

#ifdef CONFIG_OF
/* Update the name with proper device name (wifi-qcaxxxx) */
const struct of_device_id ath_wifi_of_match[] = {
    {.compatible = "qca,wifi-ipq40xx", .data = (void *) &ath9k_platform_id_table[0] },
    { /*sentinel*/},
};
#endif

static int ath_ahb_probe(struct platform_device *pdev)
{
	const struct platform_device_id *id;
	int ret;
#ifdef CONFIG_OF
    const struct of_device_id *of_id = NULL;
#endif

#ifdef CONFIG_OF
    of_id = of_match_device(ath_wifi_of_match, &pdev->dev);
    if (of_id) {
        id = of_id->data;
        if (id->driver_data == IPQ4019_DEVICE_ID) {
            return ol_ath_ahb_probe(pdev, id);
        }
    } else
#endif
    id = platform_get_device_id(pdev);

    /* Do not register Direct attach devices if ahbskip is 1 */
    if(ahbskip) {
        ret = -EINVAL;
        goto out;
    }

	spdev = pdev;

    /* Assuming platform_data will be populated in case of non OF devices */
#ifdef CONFIG_OF
    if (!of_id)
    {
#endif
        if (!pdev->dev.platform_data) {
            printk("no platform data specified\n");
            ret = -EINVAL;
            goto out;
        }
#ifdef CONFIG_OF
    }
#endif

	ret = init_ath_wmac(id->driver_data, 0);
out:
	return ret;
}

static int ath_ahb_remove(struct platform_device *pdev)
{
#ifdef CONFIG_OF
    const struct of_device_id *of_id = NULL;
	const struct platform_device_id *id;
#endif

	spdev = NULL;

#ifdef CONFIG_OF
    of_id = of_match_device(ath_wifi_of_match, &pdev->dev);
    if (of_id) {
        id = of_id->data;
        if (id->driver_data == IPQ4019_DEVICE_ID) {
            ol_ath_ahb_remove(pdev);
            return 0;
        }
    }
#endif
	exit_ath_wmac(0);
	return 0;
}

static struct platform_driver ath_ahb_driver = {
	.probe      = ath_ahb_probe,
	.remove     = ath_ahb_remove,
	.driver		= {
		.name	= "ath10k_ahb",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = ath_wifi_of_match,
#endif
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
