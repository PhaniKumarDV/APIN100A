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
 */

#ifndef _IPQ40XX_PCM_H_
#define _IPQ40XX_PCM_H_

#include <linux/sound.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include "ipq40xx-mbox.h"

struct ipq40xx_pcm_rt_priv {
	int channel;
	struct device *dev;
	struct ipq40xx_mbox_desc *last_played;
	unsigned int processed_size;
	uint32_t period_size;
	uint32_t curr_pos;
	int mmap_flag;
};

#endif /* _IPQ40XX_PCM_H_ */
