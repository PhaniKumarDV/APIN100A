/* Copyright (c) 2015, The Linux Foundation. All rights reserved.
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
 *
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/usb/phy.h>
#include <linux/reset.h>
#include <linux/of_device.h>

/**
 *  USB Hardware registers
 */
#define PHY_CTRL0_ADDR	0x000
#define PHY_CTRL1_ADDR	0x004
#define PHY_CTRL2_ADDR	0x008
#define PHY_CTRL3_ADDR	0x00C
#define PHY_CTRL4_ADDR	0x010
#define PHY_MISC_ADDR	0x024
#define PHY_IPG_ADDR	0x030

#define PHY_CTRL0_EMU_ADDR	0x180
#define PHY_CTRL1_EMU_ADDR	0x184
#define PHY_CTRL2_EMU_ADDR	0x188
#define PHY_CTRL3_EMU_ADDR	0x18C
#define PHY_CTRL4_EMU_ADDR	0x190
#define PHY_MISC_EMU_ADDR	0x1A4
#define PHY_IPG_EMU_ADDR	0x1B0

#define PHY_CTRL0_VAL	0xA4600015
#define PHY_CTRL1_VAL	0x09500000
#define PHY_CTRL2_VAL	0x00058180
#define PHY_CTRL3_VAL	0x6DB6DCD6
#define PHY_CTRL4_VAL	0x836DB6DB
#define PHY_MISC_VAL	0x3803FB0C
#define PHY_IPG_VAL		0x47323232

#define PHY_CTRL0_EMU_VAL	0xb4000015
#define PHY_CTRL1_EMU_VAL	0x09500000
#define PHY_CTRL2_EMU_VAL	0x00058180
#define PHY_CTRL3_EMU_VAL	0x6DB6DCD6
#define PHY_CTRL4_EMU_VAL	0x836DB6DB
#define PHY_MISC_EMU_VAL	0x3803FB0C
#define PHY_IPG_EMU_VAL		0x47323232

#define USB30_HS_PHY_HOST_MODE	(0x01 << 21)
#define USB20_HS_PHY_HOST_MODE	(0x01 << 5)

/* used to differentiate between USB3 HS and USB2 HS PHY */
struct qca_baldur_hs_data {
	unsigned int usb3_hs_phy;
	unsigned int phy_config_offset;
};

struct qca_baldur_hs_phy {
	struct device *dev;
	struct usb_phy phy;

	void __iomem *base;

	struct reset_control *por_rst;
	struct reset_control *srif_rst;

	unsigned int host;
	unsigned int emulation;
	const struct qca_baldur_hs_data *data;
};

#define	phy_to_dw_phy(x)	container_of((x), struct qca_baldur_hs_phy, phy)

/**
 * Write register.
 *
 * @base - PHY base virtual address.
 * @offset - register offset.
 * @val - value to write.
 */
static inline void qca_baldur_hs_write(void __iomem *base, u32 offset, u32 val)
{
	writel(val, base + offset);
}

/**
 * Read register.
 *
 * @base - PHY base virtual address.
 * @offset - register offset.
 */
static inline u32 qca_baldur_hs_read(void __iomem *base, u32 offset)
{
	u32 val;

	val = readl(base + offset);
	return val;
}

/**
 * Write register and read back masked value to confirm it is written
 *
 * @base - PHY base virtual address.
 * @offset - register offset.
 * @mask - register bitmask specifying what should be updated
 * @val - value to write.
 */
static inline void qca_baldur_hs_write_readback(void __iomem *base, u32 offset,
		const u32 mask, u32 val)
{
	u32 write_val, tmp = readl(base + offset);

	tmp &= ~mask;		/* retain other bits */
	write_val = tmp | val;

	writel(write_val, base + offset);

	/* Read back to see if val was written */
	tmp = readl(base + offset);
	tmp &= mask;		/* clear other bits */

	if (tmp != val)
		pr_err("write: %x to BALDUR PHY: %x FAILED\n", val, offset);
}

static int qca_baldur_phy_read(struct usb_phy *x, u32 reg)
{
	struct qca_baldur_hs_phy *phy = phy_to_dw_phy(x);

	return qca_baldur_hs_read(phy->base, reg);
}

static int qca_baldur_phy_write(struct usb_phy *x, u32 val, u32 reg)
{
	struct qca_baldur_hs_phy *phy = phy_to_dw_phy(x);

	qca_baldur_hs_write(phy->base, reg, val);
	return 0;
}

static int qca_baldur_hs_phy_init(struct usb_phy *x)
{
	struct qca_baldur_hs_phy	*phy = phy_to_dw_phy(x);

	/* assert HS PHY POR reset */
	reset_control_assert(phy->por_rst);
	msleep(10);

	/* assert HS PHY SRIF reset */
	reset_control_assert(phy->srif_rst);
	msleep(10);

	/* deassert HS PHY SRIF reset and program HS PHY registers */
	reset_control_deassert(phy->srif_rst);
	msleep(10);

	if (!phy->emulation) {
		/* perform PHY register writes */
		qca_baldur_hs_write(phy->base, PHY_CTRL0_ADDR, PHY_CTRL0_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL1_ADDR, PHY_CTRL1_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL2_ADDR, PHY_CTRL2_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL3_ADDR, PHY_CTRL3_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL4_ADDR, PHY_CTRL4_VAL);
		qca_baldur_hs_write(phy->base, PHY_MISC_ADDR, PHY_MISC_VAL);
		qca_baldur_hs_write(phy->base, PHY_IPG_ADDR, PHY_IPG_VAL);
	} else {
		/* perform PHY register writes */
		qca_baldur_hs_write(phy->base, PHY_CTRL0_EMU_ADDR, PHY_CTRL0_EMU_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL1_EMU_ADDR, PHY_CTRL1_EMU_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL2_EMU_ADDR, PHY_CTRL2_EMU_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL3_EMU_ADDR, PHY_CTRL3_EMU_VAL);
		qca_baldur_hs_write(phy->base, PHY_CTRL4_EMU_ADDR, PHY_CTRL4_EMU_VAL);
		qca_baldur_hs_write(phy->base, PHY_MISC_EMU_ADDR, PHY_MISC_EMU_VAL);
		qca_baldur_hs_write(phy->base, PHY_IPG_EMU_ADDR, PHY_IPG_EMU_VAL);
	}

	msleep(10);

	/* de-assert USB3 HS PHY POR reset */
	reset_control_deassert(phy->por_rst);

	return 0;
}

static int qca_baldur_hs_get_resources(struct qca_baldur_hs_phy *phy)
{
	struct platform_device *pdev = to_platform_device(phy->dev);
	struct resource *res;
	struct device_node *np = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	phy->base = devm_ioremap_resource(phy->dev, res);
	if (IS_ERR(phy->base))
		return PTR_ERR(phy->base);

	phy->por_rst = devm_reset_control_get(phy->dev, "por_rst");
	if (IS_ERR(phy->por_rst))
		return PTR_ERR(phy->por_rst);

	phy->srif_rst = devm_reset_control_get(phy->dev, "srif_rst");
	if (IS_ERR(phy->srif_rst))
		return PTR_ERR(phy->srif_rst);

	np = of_node_get(pdev->dev.of_node);
	if (of_property_read_u32(np, "qca,host", &phy->host)
			|| of_property_read_u32(np, "qca,emulation", &phy->emulation)) {
		pr_err("%s: error reading critical device node properties\n", np->name);
		return -EFAULT;
	}

	return 0;
}

static void qca_baldur_hs_put_resources(struct qca_baldur_hs_phy *phy)
{
	reset_control_assert(phy->srif_rst);
	reset_control_assert(phy->por_rst);
}

static int qca_baldur_hs_remove(struct platform_device *pdev)
{
	struct qca_baldur_hs_phy *phy = platform_get_drvdata(pdev);

	usb_remove_phy(&phy->phy);
	return 0;
}

static void qca_baldur_hs_phy_shutdown(struct usb_phy *x)
{
	struct qca_baldur_hs_phy   *phy = phy_to_dw_phy(x);

	qca_baldur_hs_put_resources(phy);
}

static struct usb_phy_io_ops qca_baldur_io_ops = {
	.read = qca_baldur_phy_read,
	.write = qca_baldur_phy_write,
};

static const struct qca_baldur_hs_data usb3_hs_data = {
	.usb3_hs_phy = 1,
	.phy_config_offset = USB30_HS_PHY_HOST_MODE,
};

static const struct qca_baldur_hs_data usb2_hs_data = {
	.usb3_hs_phy = 0,
	.phy_config_offset = USB20_HS_PHY_HOST_MODE,
};

static const struct of_device_id qca_baldur_hs_id_table[] = {
	{ .compatible = "qca,baldur-usb3-hsphy", .data = &usb3_hs_data },
	{ .compatible = "qca,baldur-usb2-hsphy", .data = &usb2_hs_data },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, qca_baldur_hs_id_table);

static int qca_baldur_hs_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct qca_baldur_hs_phy *phy;
	int err;

	match = of_match_device(qca_baldur_hs_id_table, &pdev->dev);
	if (!match)
		return -ENODEV;

	phy = devm_kzalloc(&pdev->dev, sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	platform_set_drvdata(pdev, phy);
	phy->dev = &pdev->dev;

	phy->data = match->data;

	err = qca_baldur_hs_get_resources(phy);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to request resources: %d\n", err);
		return err;
	}

	phy->phy.dev		= phy->dev;
	phy->phy.label		= "qca-baldur-hsphy";
	phy->phy.init		= qca_baldur_hs_phy_init;
	phy->phy.shutdown	= qca_baldur_hs_phy_shutdown;
	phy->phy.type		= USB_PHY_TYPE_USB2;
	phy->phy.io_ops		= &qca_baldur_io_ops;

	err = usb_add_phy_dev(&phy->phy);
	return err;
}

static struct platform_driver qca_baldur_hs_driver = {
	.probe		= qca_baldur_hs_probe,
	.remove		= qca_baldur_hs_remove,
	.driver		= {
		.name	= "qca-baldur-hsphy",
		.owner	= THIS_MODULE,
		.of_match_table = qca_baldur_hs_id_table,
	},
};

module_platform_driver(qca_baldur_hs_driver);

MODULE_ALIAS("platform:qca-baldur-hsphy");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("USB3 QCA BALDUR HSPHY driver");
