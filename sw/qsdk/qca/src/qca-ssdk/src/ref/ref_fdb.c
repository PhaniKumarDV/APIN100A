/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal_misc.h"
#include "fal_mib.h"
#include "fal_port_ctrl.h"
#include "fal_portvlan.h"
#include "fal_fdb.h"
#include "fal_stp.h"
#include "fal_igmp.h"
#include "fal_qos.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//#include <asm/mach-types.h>
#include <generated/autoconf.h>
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#else
#include <net/switch.h>
#include <linux/ar8216_platform.h>
#endif
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include "ssdk_plat.h"
#include "ref_vlan.h"


int
qca_ar8327_sw_atu_flush(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	/* 0: dynamic 1:dynamic, static */
	fal_fdb_del_all(0, 1);

	return 0;
}

int
qca_ar8327_sw_atu_dump(struct switch_dev *dev,
		       		const struct switch_attr *attr,
		       		struct switch_val *val)
{
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);
	a_uint32_t rv;
	char *buf;
	a_uint32_t len = 0;
	a_uint32_t i = 0;
	fal_fdb_op_t option;
	fal_fdb_entry_t entry;

	buf = (char*) priv->buf;
	memset(buf, 0, 2048);
	memset(&option, 0, sizeof(fal_fdb_op_t));
	memset(&entry, 0, sizeof(fal_fdb_entry_t));

	if (priv->version == QCA_VER_AR8227)
		rv = fal_fdb_first(0, &entry);
	else
		rv = fal_fdb_extend_first(0, &option, &entry);
	while (!rv)
    {
		len += snprintf(buf+len, 2048-len, "MAC: %02x:%02x:%02x:%02x:%02x:%02x PORTMAP: 0x%02x VID: 0x%x STATUS: 0x%x\n",
			entry.addr.uc[0],entry.addr.uc[1],entry.addr.uc[2],entry.addr.uc[3],entry.addr.uc[4],entry.addr.uc[5],
			entry.port.map,
			entry.fid,
			entry.static_en);

		if (2048-len < 74){
//			snprintf(buf+len, 2048-len, "Buffer not enough!\n");
			break;
		}
		if (priv->version == QCA_VER_AR8227)
			rv = fal_fdb_iterate(0, &i, &entry);
		else
			rv = fal_fdb_extend_next(0, &option, &entry);
    }

	val->value.s = (char*)(priv->buf);
	val->len = len;

	return 0;
}

#define MAX_PORT 6
/*
 * example:
 * vid=4;
 * char addr[6] = {0x00, 0x01, 0x02, 0x88, 0x00, 0xaa}
 *
 * return value:
 *  success: 0 - 5
 *  fail: 0xffffffff
 */
fal_port_t
ref_fdb_get_port_by_mac(unsigned int vid, const char * addr)
{
	fal_fdb_entry_t entry = {0};
	unsigned char i;
	sw_error_t rv;

	entry.fid = vid;
	for (i = 0; i < 6; i++)
		entry.addr.uc[i] = addr[i];

	rv = fal_fdb_find(0, &entry);
	if (rv != SW_OK)
		return 0xffffffff;

	for (i = 0; i < MAX_PORT; i++)
	{
		if (entry.port.id & (0x1 << i))
		{
			return i;
		}
	}
	return 0xffffffff;
}

EXPORT_SYMBOL(ref_fdb_get_port_by_mac);

