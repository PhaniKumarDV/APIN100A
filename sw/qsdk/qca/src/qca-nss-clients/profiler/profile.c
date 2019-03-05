/*
 **************************************************************************
 * Copyright (c) 2014,2016 The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

/*
 * qsdk/qca/src/qca-nss-drv/profiler/profile.c
 *
 *   Implementation for NetAP Profiler
 */

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/fs.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/thread_info.h>
#include <linux/ctype.h>
#include <nss_api_if.h>

#include "profilenode.h"
#include "profpkt.h"

/*
 * This is the driver for the NetAP Core profiler. The system interface to the driver is
 *	profile_register_performance_counter(), defined in <asm/profile.>
 *	a set of proc files (proc/profile/<*>), used by the profiler daemon
 *
 * communication between the profiler components is described in a set of header files.
 * There are multiple versions of these files that must be kept synchronized:
 *	in nss/source/pkg/profile
 *	in tools/profiler
 *	in qsdk/qca/src/qca-nss-drv/profiler
 *
 * profilesample.h specifies the sample format used by pkg/profile, profile driver, and ip3kprof (two versions)
 * profilenode.h specifies the driver node communication between NetAP and the profile driver. (two versions)
 * profpkt.h specifies the network packet format between the profile driver, profile daemon, and ip3kprof (two versions)
 *
 *
 * NSS profile sampler:
 *	pkg/profile/src/profile.c
 *	pkg/profile/include/profilenode.h
 *	pkg/profile/include/profilesample.h
 *
 * profile driver: this code
 *	qsdk/qca/src/qca-nss-drv/profiler
 *
 * profilerd: the user daemon that sends data to the tool
 *	qsdk/qca/feeds/qca/utils/profilerd
 *
 * ubicom32-prof: the Windows tool
 *	tools/profiler/src/(many files)
 *
 */

#ifdef	PROFILE_DEBUG
#define	profileDebug(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define	profileInfo(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define	profileWarn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define	profileDebug(s, ...)
#define	profileInfo(s, ...)
#define	profileWarn(s, ...)
#endif

static void profiler_handle_reply(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm);

/*
 * LINUX and Ultra counters must all fit in one packet
 */
#define PROFILE_LINUX_MAX_COUNTERS 40
#define PROFILE_STS_EVENT_COUNTERS 8
#define PROFILE_STS_EVENT_THREAD_BITS 5

static int profile_num_counters = 0;
static volatile unsigned int *profile_counter[PROFILE_LINUX_MAX_COUNTERS];
static char profile_name[PROFILE_LINUX_MAX_COUNTERS][PROFILE_COUNTER_NAME_LENGTH];

/*
 * internal function to check if @name has been registered before
 * return the found index, or -1 otherwise
 */
static int __profile_find_entry(char *name)
{
	int i;

	for (i = 0; i < profile_num_counters; i++) {
		if (!strncasecmp(name, profile_name[i], PROFILE_COUNTER_NAME_LENGTH)) {
			return i;
		}
	}
	return -1;
}

/*
 * profile_register_performance_counter - register @counter into profile tracking list by key @name
 * @counter: pointer of the counter variable
 * @name: identifier of this counter
 *
 * Returns zero if total entries exceeding PROFILE_LINUX_MAX_COUNTERS
 * non-zero otherwise.
 *
 * Each @name gives unique entry for @counter, by allocating a new array slot or just use existing one.
 * No need of de-registration API, since a loadable module's new insmod, will replace the
 * @counter's * new address at the same profile_counter[] slot.
 */
int profile_register_performance_counter(volatile unsigned int *counter, char *name)
{
	int i;

	if (profile_num_counters >= PROFILE_LINUX_MAX_COUNTERS) {
		return 0;
	}

	i = __profile_find_entry(name);
	if (i < 0) {
		i = profile_num_counters++;
	}

	profile_counter[i] = counter;
	strlcpy(profile_name[i], name, PROFILE_COUNTER_NAME_LENGTH);
	profile_name[i][PROFILE_COUNTER_NAME_LENGTH - 1] = 0;

	return 1;
}

/*
 * make a packet full of sample data
 */
static int profile_make_data_packet(char *buf, int blen, struct profile_io *pn)
{
	int sp_samples = 0;	/* separated samples if any */
	int ns;		/* number of samples requested */
	struct profile_header ph;
	struct profile_sample_ctrl_header *psc_hd = &pn->pnc.pn2h->psc_header;

	if (blen < sizeof(ph) + sizeof(struct profile_sample)) {
		return -EINVAL;
	}

	profileDebug("%p stat %x cnt %d %p\n", pn->pnc.pn2h, pn->pnc.pn2h->mh.md_type, psc_hd->count, pn->ccl);

	if (pn->pnc.pn2h->mh.md_type == PINGPONG_EMPTY || psc_hd->count < 1) {
		struct profile_n2h_sample_buf *nsb;
		ns = (pn->ccl_read + 1) & (CCL_SIZE-1);
		nsb = pn->ccl + ns;
		if (ns == pn->ccl_write || nsb->mh.md_type != PINGPONG_FULL) {
			profileInfo("%s: waiting more data %x %p : ns %d rd %d wr %d\n", __func__, nsb->mh.md_type, nsb, ns, pn->ccl_read, pn->ccl_write);
			return -EAGAIN;
		}
		pn->ccl_read = ns;
		profileInfo("sp %p => %p rd %d %p\n", pn->pnc.samples, nsb->samples, ns, nsb);
		psc_hd = &nsb->psc_header;
		pn->pnc.pn2h = nsb;
		pn->pnc.samples = nsb->samples;
		pn->pnc.cur = 0;
	}
	pn->pnc.pn2h->mh.md_type = PINGPONG_INUSE;

	/*
	 * fill in the packet header
	 */
	memset(&ph, 0, sizeof(ph));
	ph.pph.magic = htons(PROF_MAGIC + PROFILE_VERSION);
	ph.pph.header_size = sizeof(ph);
	ph.pph.profile_instructions = 0;
	ph.pph.clock_freq = pn->pnc.un.cpu_freq;
	ph.pph.ddr_freq = pn->pnc.un.ddr_freq;
	ph.pph.cpu_id = pn->pnc.un.cpu_id;
	ph.pph.seq_num = htonl(pn->profile_sequence_num);
	ph.pph.sample_stack_words = PROFILE_STACK_WORDS;

	ns = (blen - sizeof(ph)) / sizeof(struct profile_sample);
	profileInfo("%X: blen %d ns = %d psc_hd count %d ssets %d phs %d pss %d\n", pn->profile_sequence_num, blen, ns, psc_hd->count, psc_hd->exh.sample_sets, sizeof(ph), sizeof(struct profile_sample));
	if (ns > psc_hd->count)
		ns = psc_hd->count;
	if (ns == 0) {
		printk("NS should not be 0: rlen %d hd cnt %d\n", blen, psc_hd->count);
		return 0;
	}

	/*
	 * if buf cannot hold all samples, then samples must be separated by set.
	 */
	if (ns < psc_hd->count) {
		ph.exh.sets_map = psc_hd->exh.sets_map;	/* save for separating sets */
		do {
			sp_samples += psc_hd->exh.sets_map & 0x0F;
			psc_hd->exh.sets_map >>= 4;	/* remove the last set */
			psc_hd->exh.sample_sets--;
			ph.exh.sample_sets++;		/* save for restore later */
		} while ((psc_hd->count - sp_samples) > ns);
		ns = psc_hd->count - sp_samples;
	}
	ph.pph.sample_count = ns;
	if (copy_to_user(buf, &ph.pph, sizeof(ph.pph)) != 0) {
		return -EFAULT;
	}
	buf += sizeof(ph.pph);

	/*
	 *	ph.exh is unused dummy; and psc_hd->exh is used directly to avoid double mem copy
	 */
	if (copy_to_user(buf, &psc_hd->exh, sizeof(psc_hd->exh)) != 0) {
		return -EFAULT;
	}
	buf += sizeof(psc_hd->exh);

	blen = ns * sizeof(struct profile_sample);
	profileDebug("-profile_make_data_packet %p slen %d cur %d dcped %d + %d\n", pn->pnc.samples, blen, pn->pnc.cur, sizeof(ph.pph), sizeof(psc_hd->exh));
	if (copy_to_user(buf, &pn->pnc.samples[pn->pnc.cur], blen) != 0) {
		return -EFAULT;
	}
	pn->pnc.cur += ns;
	psc_hd->count -= ns;
	if (psc_hd->count < 1)
		pn->pnc.pn2h->mh.md_type = PINGPONG_EMPTY;

	/*
	 * restore left over sample counts; 0s for no one
	 */
	if (sp_samples) {
		profileDebug("%d sps %d %d: sets %d : %d map %x <> %x\n", psc_hd->count, ns, sp_samples, psc_hd->exh.sample_sets, ph.exh.sample_sets, psc_hd->exh.sets_map, ph.exh.sets_map);
		psc_hd->exh.sample_sets = ph.exh.sample_sets;
		psc_hd->exh.sets_map = ph.exh.sets_map;
	}

	pn->profile_sequence_num++;
	blen += sizeof(ph);
	profileDebug("+profile_make_data_packet %d phd len %d nsp %p rd %d cnt %d\n", blen, sizeof(ph), pn->pnc.pn2h, pn->ccl_read, psc_hd->count);
	return blen;
}

/*
 * This is no longer needed due to NetAP and Linux use different CPUs, and profile is NetAP only.
 * All related code will be removed after corresponging code in visual tool is corrected; otherwise
 * visual tool will mis-behave
 */
struct profile_counter profile_builtin_stats[] =
{
	{
	"Free memory(KB)", 0
	},
	{
	"Max free Block(KB)", 0
	}
};

/*
 * make a packet full of performance counters (software)
 */
static int profile_make_stats_packet(char *buf, int bytes, struct profile_io *pn)
{
	static char prof_pkt[PROFILE_MAX_PACKET_SIZE];

	char *ptr;
	int n;
	struct profile_counter *counter_ptr;
	struct profile_header_counters *hdr = (struct profile_header_counters *)prof_pkt;
	struct profile_sample_ctrl_header *psc_hd = &pn->pnc.pn2h->psc_header;

	if (bytes > PROFILE_MAX_PACKET_SIZE) {
		bytes = PROFILE_MAX_PACKET_SIZE;
	}
	n = sizeof(profile_builtin_stats) + (pn->pnc.un.num_counters + profile_num_counters) * sizeof(*counter_ptr);

	if ((bytes - sizeof(hdr)) < n) {
		profileWarn("room too small %d for cnts %d\n", bytes, n);
		return 0;
	}

	hdr->magic = htons(PROF_MAGIC_COUNTERS);
	hdr->ultra_count = htons(pn->pnc.un.num_counters);
	hdr->linux_count = htonl(profile_num_counters + sizeof(profile_builtin_stats) / sizeof(*counter_ptr));
	hdr->ultra_sample_time = psc_hd->exh.clocks;
	hdr->linux_sample_time = psc_hd->exh.clocks; /* QSDK has no time func */

	n = pn->pnc.un.num_counters;	/* copy NSS counters */
	n *= sizeof(pn->pnc.un.counters[0]);
	ptr = (char*) (hdr + 1);
	memcpy(ptr, (void *)(pn->pnc.un.counters), n);
	ptr += n;

	counter_ptr = (struct profile_counter *)ptr;
	for (n = 0; n < profile_num_counters; ++n) {
		counter_ptr->value = htonl(*profile_counter[n]);
		strlcpy(counter_ptr->name, profile_name[n],
			PROFILE_COUNTER_NAME_LENGTH);
		counter_ptr++;
	}
	ptr = (char*)counter_ptr;

	/*
	 * built in statistics
	profile_get_memory_stats(&total_free, &max_free);
	 */
	profile_builtin_stats[0].value = 0;
	profile_builtin_stats[1].value = 0;
	memcpy(ptr, (void *)profile_builtin_stats, sizeof(profile_builtin_stats));
	ptr += sizeof(profile_builtin_stats);

	n = ptr - prof_pkt;
	if (copy_to_user(buf, prof_pkt, n) != 0) {
		return -EFAULT;
	}
	return n;
}

/*
 * space for all memory blocks so we can hold locks for short time when walking tables
 */
static struct profile_io *node[NSS_MAX_CORES];

static int profile_open(struct inode *inode, struct file *filp)
{
	int	n;
	struct profile_io *pn;

	if (filp->private_data)
		printk(KERN_WARNING "%s: %p\n", filp->f_path.dentry->d_iname, filp->private_data);

	n = filp->f_path.dentry->d_iname[strlen(filp->f_path.dentry->d_iname) - 1] - '0';
	if (n < 0 || n >= NSS_MAX_CORES)
		n = 0;
	pn = node[n];
	if (!pn) {
		return -ENOENT;
	}

	if (!pn->pnc.enabled && nss_get_state(pn->ctx) == NSS_STATE_INITIALIZED) {
		nss_tx_status_t ret;

		/*
		 * sw_ksp_ptr is used as event flag. NULL means normal I/O
		 */
		pn->sw_ksp_ptr = NULL;
		pn->pnc.enabled = 1;
		pn->profile_first_packet = 1;
		pn->pnc.un.hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_START_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
				sizeof(pn->pnc.un), profiler_handle_reply, pn);
		profileInfo("%s: %d -- %p: ccl %p sp %p\n", __func__, ret,
			pn, pn->ccl, pn->pnc.samples);
		filp->private_data = pn;
		return 0;
	}

	profileWarn("profile ena %d nss stat %x\n", pn->pnc.enabled,
			nss_get_state(pn->ctx));
	return -EBUSY;
}

/*
 * return a udp packet ready to send to the profiler tool
 * when there are no packets left to make, return 0
 */
static int profile_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int result = 0;
	int slen = 0;
	struct profile_io *pn = (struct profile_io *)filp->private_data;
	if (!pn) {
		return -ENOENT;
	}

	if (!pn->pnc.enabled) {
		return -EPERM;
	}
	if (pn->sw_ksp_ptr) {
		struct debug_box *db = (struct debug_box *) pn->sw_ksp_ptr;
		slen = (PROFILE_STS_EVENT_COUNTERS + 1) * sizeof(db->data[0]);
		if (copy_to_user(buf, db->data, slen))
			return -EFAULT;
		return	slen;
	}

	if (!pn->pnc.samples) {
		return -ENOMEM;
	}

	if (pn->profile_first_packet) {
		result = profile_make_stats_packet(buf, count, pn);
		pn->profile_first_packet = 0;
		profileInfo("%d profile_make_stats_packet %d\n", result, count);

#ifdef	PROFILE_SEP_STAT
		/*
		 * currectly, stat and sample data are combined in one pkt for efficient;
		 * but this is harder to debug and required remote tool to handle
		 * packet in all-in-one method instead of individual handler.
		 */
		return result;
#endif
	}

	if (result > 0) {
		buf += result;
		count -= result;
		slen = result;
	}
	result = profile_make_data_packet(buf, count, pn);
	if (result == 0) {
		pn->profile_first_packet = 1;
	}
	profileInfo("%d: profile_make_data_packet %d %d\n", result, count, slen);

	profileInfo("%d: read\n", pn->pnc.enabled);
	if (pn->pnc.enabled < 0) {
		nss_tx_status_t ret;
		pn->pnc.enabled = 1;
		pn->pnc.un.hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_START_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un, sizeof(pn->pnc.un),
						profiler_handle_reply, pn);
		profileWarn("%s: restart %d -- %p: ccl %p sp %p\n", __func__,
				ret, pn, pn->ccl, pn->pnc.samples);
	}

	return result + slen;
}

/*
 * the close function paired with profiler_open
 */
static int profile_release(struct inode *inode, struct file *filp)
{
	struct profile_io *pn = (struct profile_io *)filp->private_data;
	if (!pn) {
		return -ENOENT;
	}

	if (pn->pnc.enabled) {
		nss_tx_status_t ret;
		pn->sw_ksp_ptr = NULL;
		pn->pnc.enabled = 0;
		pn->pnc.un.hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_STOP_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
				sizeof(pn->pnc.un), profiler_handle_reply, pn);
		profileInfo("%s: %p %d\n", __func__, pn, ret);
		return 0;
	}
	profileWarn("%s: attempt closing non-open dev %p\n", __func__, pn);
	pn->profile_first_packet = 1;
	return -EBADF;
}

/*
 * profiler_handle_stat_event_reply()
 *	print current FW stat event counter configurations
 */
static void profiler_handle_stat_event_reply(struct nss_ctx_instance *nss_ctx,
						struct nss_cmn_msg *ncm)
{
	struct profile_io *pio = (struct profile_io *) ncm->app_data;
	struct debug_box *pdb = (struct debug_box *) &pio->pnc;
	struct debug_box *db = (struct debug_box *) &ncm[1];
	int i, thrds;

	for (i = 0; i < db->dlen; i++)
		printk("stat counter %d: %x\n", i, db->data[i]);

	thrds = db->data[i];
	i = (1 << PROFILE_STS_EVENT_THREAD_BITS) - 1;
	profileInfo("%d: event end mark %x, ThrA %d ThrB %d\n",
		ncm->len, thrds, (thrds & i) + 1,
		((thrds >> PROFILE_STS_EVENT_THREAD_BITS) & i) + 1);

	/*
	 * save data for read()
	 */
	memcpy(pdb->data, db->data, (db->dlen + 1) * sizeof(db->data[0]));
}

/*
 * parse_sys_stat_event_req()
 *	process FW stat events request: event#1 index#1 event#2 index#2 ...
 */
static int parse_sys_stat_event_req(const char *buf, size_t count,
				struct debug_box *db, struct profile_io *pio)
{
	char *cp;
	int result;

	printk("%d cmd buf %s\n", count, buf);
	if (count < 19) /* minimum data for sys_stat_event request */
		return	-EINVAL;

	if (strncmp(buf, "get-sys-stat-events", 19) == 0) {
		db->hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_GET_SYS_STAT_EVENT;
		result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un,
					sizeof(pio->pnc.un),
					profiler_handle_stat_event_reply, pio);
		profileInfo("get_sys_stat_events: %d\n", result);
		return	result == NSS_TX_SUCCESS ? count : -EFAULT;
	}

	if (strncmp(buf, "set-sys-stat-events", 19)) {
		printk("unknow event: %s\n", buf);
		return	-EINVAL;
	}

	db->dlen = sizeof(pio->pnc.un);
	memset(db->data, 0, PROFILE_STS_EVENT_COUNTERS * sizeof(db->data[0]));

	cp = strchr(buf, ' ');
	if (!cp) {
		printk("no enough paramters %s\n", buf);
		return	-EINVAL;
	}

	do {
		int idx, event;

		while (isspace(*cp))
			cp++;
		if (kstrtoul(cp, 0, (unsigned long *)&event))
			return -EINVAL;

		cp = strchr(cp, ' ');
		if (!cp) {
			printk("missing index %s\n", buf);
			return	-EINVAL;
		}
		while (isspace(*cp))
			cp++;
		idx = event >> 16;
		if (idx) {
			if ((event & 0x1FF) < 50) {
				printk("thr ID (%d) ignored for event %d\n",
					idx, event & 0x1FF);
			} else if (idx > 12) {
				if ((idx >>= 5) > 12) {
					printk("tID %d too big [1..12]\n", idx);
					return	-E2BIG;
				}
			}
		}

		if (kstrtoul(cp, 10, (unsigned long *)&idx) || idx < 0 || idx > 7) {
			printk("index %d out of range [0..7]\n", idx);
			return	-ERANGE;
		}
		printk("%p: e %d i %d\n", db, event, idx);
		db->data[idx] = event;
		cp = strchr(cp, ' ');
	} while (cp);
	db->hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_SET_SYS_STAT_EVENT;
	result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un, sizeof(pio->pnc.un),
				profiler_handle_stat_event_reply, pio);
	profileInfo("%p: %d send cmd %x to FW ret %d\n",
			db, count, db->hd_magic, result);
	return	count;
}

/*
 * parseDbgData()
 *	parsing debug requests: base_address [options] cmd length
 *
 * cmd is either read or write
 * option is one of mio, moveio, h [heap security verify], etc.
 */
static int parseDbgData(const char *buf, size_t count, struct debug_box *db)
{
	char *cp;
	int n;

	printk("%p: buf (%s) cnt %d\n", buf, buf, count);
	if (sscanf(buf, "%x", (uint32_t *)&db->base_addr) != 1) {
		printk("%s: cannot get base addr\n", __func__);
		return	-EINVAL;
	}

	cp = strchr(buf, ' ');
	if (!cp) {
noea:		printk("%s: no enough arguments\n", __func__);
		return -EFAULT;
	}

	while (isspace(*cp)) cp++;
	if (!strncmp(cp, "mio", 3) || !strncmp(cp, "moveio", 6)) {
		printk("%p: cp (%s)\n", cp, cp);
		cp = strchr(cp, ' ');
		if (!cp) {
			goto noea;
		}
		db->opts |= DEBUG_OPT_MOVEIO;
	}

	while (isspace(*cp)) cp++;
	printk("base addr %p -- %s\n", db->base_addr, cp);

	if (!strncmp(cp, "read", 4)) {
		cp = strchr(cp, ' ');
		if (cp) {
			while (isspace(*cp)) cp++;
			sscanf(cp, "%x", &db->dlen);
		}
		return 0;
	}

	n = 0;
	do {
		while (isspace(*cp)) cp++;
		if (sscanf(cp, "%x", db->data+n) != 1) {
			printk("n %d : %s\n", n, cp);
			break;
		}
		printk("write %x to off %x\n", db->data[n], n * sizeof(db->data[0]));
		n++;
		cp = strchr(cp, ' ');
	} while (cp && n < MAX_DB_WR);
	return n;
}

/*
 * display memory content read from Phy addr
 */
static void debug_if_show(struct debug_box *db, int buf_len)
{
	int i;

	for (i=0; i < db->dlen; i++) {
		if ((i & 3) == 0)
			printk("\n%p: ", db->base_addr + i);
		printk("%9x", db->data[i]);
	}
	printk("\ndumped %d (extra 1) blen %d\n", db->dlen, buf_len);
}

/*
 * show debug message we requested from NSS
 */
static void profiler_handle_debug_reply(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm)
{
	debug_if_show((struct debug_box*)&ncm[1], ncm->len);
}

/*
 * a generic Krait <--> NSS debug interface
 */
static int debug_if(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int result;
	struct debug_box *db;
	struct profile_io *pio = (struct profile_io *)filp->private_data;

	if (!pio) {
		return -ENOENT;
	}

	if (!pio->pnc.enabled) {
		return -EPERM;
	}

	db = (struct debug_box *) &pio->pnc;
	db->dlen = db->opts = 0;

	if (!isdigit(buf[0])) {
		result = parse_sys_stat_event_req(buf, count, db, pio);

		if ((result > 0) && (filp->f_flags & O_RDWR)) {
			/*
			 * set flag so event-counter can read the data from FW
			 */
			pio->sw_ksp_ptr = (uint32_t *)db;
		}
		return	result;
	}

	result = parseDbgData(buf, count, db);
	if (result < 0) {
		return	result;
	}

	if (!result) {
		db->hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_DEBUG_RD_MSG;
	} else {
		db->hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_DEBUG_WR_MSG;
		db->dlen = result;
	}
	result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un,
			sizeof(pio->pnc.un), profiler_handle_debug_reply, pio);
	printk("dbg res %d dlen = %d opt %x\n", result, db->dlen, db->opts);
	return	count;
}

static const struct file_operations profile_fops = {
	.open		= profile_open,
	.read		= profile_read,
	.release	= profile_release,
	.write		= debug_if,
};

/*
 * showing sample status on Linux console
 */
static int profile_rate_show(struct seq_file *m, void *v)
{
	struct profile_io *pn = node[0];
	if (pn) {
		struct profile_sample_ctrl_header *psc_hd = &pn->pnc.pn2h->psc_header;
		seq_printf(m, "%d samples per second.  %d ultra, %d linux virtual counters.  %d dropped samples.  %d queued of %d max sampels.  %d sent packets.\n",
			pn->pnc.un.rate, pn->pnc.un.num_counters, profile_num_counters, psc_hd->dropped_samples, psc_hd->count, psc_hd->max_samples, pn->profile_sequence_num);
	} else {
		seq_printf(m, "Profiler is not initialized.\n");
	}
	return 0;
}

static int profile_rate_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, profile_rate_show, NULL);
}

static int profile_rate_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	*off = 0;
	return 0;
}

static const struct file_operations profile_rate_fops = {
	.open		= profile_rate_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= profile_rate_write,
};

/*
 * hex dump for debug
 */
static void kxdump(void *buf, int len, const char *who)
{
	int *ip = (int*) buf;
	int lns = len >> 5;	/* 32-B each line */
	if (lns > 4)
		lns = 4;
	printk("%p: kxdump %s: len %d\n", buf, who, len);
	do {
		printk("%x %x %x %x %x %x %x %x\n", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]);
		ip += 8;
	} while (lns--);
}

/*
 * check magic # and detect Endian.
 * negtive return means failure.
 * return 1 means need to ntoh swap.
 */
static int profiler_magic_verify(struct profile_sample_ctrl_header *psc_hd, int buf_len)
{
	int swap = 0;
	if ((psc_hd->hd_magic & UBI32_PROFILE_HD_MMASK) != UBI32_PROFILE_HD_MAGIC) {
		if ((psc_hd->hd_magic & UBI32_PROFILE_HD_MMASK_REV) != UBI32_PROFILE_HD_MAGIC_REV) {
			kxdump(psc_hd, buf_len, "bad profile packet");
			printk("bad profile packet %x : %d\n", psc_hd->hd_magic, buf_len);
			return -1;
		}
		profileDebug("Profile data in different Endian type %x\n", psc_hd->hd_magic);
		swap = 1;
		psc_hd->hd_magic = ntohl(psc_hd->hd_magic);
	}
	return swap;
}

/*
 * process profile sample data from NSS
 */
static void profile_handle_nss_data(void *arg, struct nss_profiler_msg *npm)
{
	int buf_len = npm->cm.len;
	void *buf = &npm->payload;
	struct profile_io *pn;
	struct profile_n2h_sample_buf *nsb;
	struct profile_sample_ctrl_header *psc_hd = (struct profile_sample_ctrl_header *)buf;
	int	ret, wr;
	int	swap = 0;	/* only for header and info data, not samples */

	if (buf_len < (sizeof(struct profile_session) - sizeof(struct profile_counter) * (PROFILE_MAX_APP_COUNTERS))) {
		printk("profile data packet is too small to be useful %d\n", buf_len);
		return;
	}

	swap = profiler_magic_verify(psc_hd, buf_len);
	if (swap < 0) {
		return;
	}

	pn = (struct profile_io *)arg;
	profileInfo("%s: dlen %d swap %d cmd %x - %d\n", __func__, buf_len, swap, npm->cm.type, (pn->ccl_read - pn->ccl_write) & (CCL_SIZE-1));
	//kxdump(buf, buf_len, "process profile packet");

	if (npm->cm.type == NSS_PROFILER_FIXED_INFO_MSG) {
		struct profile_session *pTx = (struct profile_session *)buf;
		if (swap) {
			pn->pnc.un.rate = ntohl(pTx->rate);
			pn->pnc.un.cpu_id = ntohl(pTx->cpu_id);
			pn->pnc.un.cpu_freq = ntohl(pTx->cpu_freq);
			pn->pnc.un.ddr_freq = ntohl(pTx->ddr_freq);
			pn->pnc.un.num_counters = pTx->num_counters;
		} else {
			pn->pnc.un = *pTx;
		}
		memcpy(pn->pnc.un.counters, pTx->counters, pn->pnc.un.num_counters * sizeof(pn->pnc.un.counters[0]));
		pn->profile_first_packet = 1;
		return;
	}

	wr = (pn->ccl_write + 1) & (CCL_SIZE-1);
	nsb = pn->ccl + wr;
	swap = (pn->ccl_read - wr) & (CCL_SIZE-1);	/* PROFILER_FLOWCTRL */
	if (nsb->mh.md_type != PINGPONG_EMPTY || (swap && swap < 5)) {
		if (pn->pnc.enabled > 0) {
			pn->pnc.enabled = -1;
			pn->pnc.un.hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_STOP_MSG;
			ret = nss_profiler_if_tx_buf(pn->ctx,
					&pn->pnc.un, sizeof(pn->pnc.un),
					profiler_handle_reply, pn);
			profileWarn("%d temp stop sampling engine %d\n", swap, ret);
		}
		if (swap < 3) {
			profileWarn("w%p.%d: %d no room for new profile samples r%p.%d\n", nsb, wr, swap, pn->ccl+pn->ccl_read, pn->ccl_read);
			return;	/* -EMSGSIZE */
		}
	}
	pn->ccl_write = wr;

	/*
	 * sampling data -- hdr NBO swap is done at NSS side via SWAPB.
	 */
	memcpy(&nsb->psc_header, buf, buf_len); /* pn->pnc.pn2h->psc_header = *psc_hd; maybe faster, but take more memory */

	nsb->mh.md_type = PINGPONG_FULL;

	/*
	 * ask for perf_counters (software counters) update every 32 samples
	 */
	if (!wr) {
		pn->pnc.un.hd_magic = UBI32_PROFILE_HD_MAGIC | NSS_PROFILER_COUNTERS_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
				sizeof(pn->pnc.un), profiler_handle_reply, pn);
		if (ret == NSS_TX_FAILURE)
			printk("req counters Cmd failed %d %d\n", ret, wr);
	}
	profileInfo("filled %p %p wr %d\n", nsb, nsb->samples, pn->ccl_write);
}

/*
 * process N2H reply for message we sent to NSS -- currently no action
 */
static void profiler_handle_reply(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm)
{
	switch (ncm->response) {
	default:
		profileWarn("%p: profiler had error response %d\n", nss_ctx, ncm->response);
		/*
		 * fail through -- no plan to do anything yet
		 */
	case NSS_CMN_RESPONSE_ACK:
		return;
	}
}

/*
 * initialize basic profile data structure
 */
static void profile_init(struct profile_io *node)
{
	int n;

	memset(&node->pnc, 0, sizeof(node->pnc));
	node->ccl_read = 0;
	node->ccl_write = -1;
	node->pnc.pn2h = node->ccl;
	node->pnc.samples = node->ccl->samples;

	for (n = 0; n < CCL_SIZE; n++) {
		node->ccl[n].mh.md_type = PINGPONG_EMPTY;
		node->ccl[n].psc_header.count = 0;
	}

	/*
	 * sw_ksp is an array of pointers to struct thread_info,
	 * the current task executing for each linux virtual processor
	node->sw_ksp_ptr = sw_ksp;
	 */
	node->sw_ksp_ptr = NULL;
	node->task_offset = offsetof(struct thread_info, task);
	node->pid_offset = offsetof(struct task_struct, tgid);
}

static struct proc_dir_entry *pdir;

/*
 * init_module cannot call exit_MODULE, so use this wrapper
 */
void netap_profile_release_resource(void)
{
	if (pdir) {
		remove_proc_entry("rate", pdir);
		remove_proc_entry("data", pdir);
		remove_proc_entry("data1", pdir);
	}
	kfree(node[0]->ccl);
	kfree(node[0]);
	node[0] = NULL;
}

/*
 * kernel module entry
 */
int __init netap_profile_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	/*
	 * we need N nodes, not one node + N ctx, for N cores
	 */
	node[0] = kmalloc(sizeof(*node[0]) * NSS_MAX_CORES, GFP_KERNEL);
	if (!node[0]) {
		printk(KERN_INFO "Profiler CTRL kmalloc failed.\n");
		return -ENOMEM;
	}

	node[0]->ccl = kmalloc(sizeof(*node[0]->ccl) * CCL_SIZE * NSS_MAX_CORES, GFP_KERNEL);
	if (!node[0]->ccl) {
		printk(KERN_INFO "Profiler n2h_sample_buf kmalloc failed.\n");
		kfree(node[0]);
		node[0] = NULL;
		return -ENOMEM;
	}

	/*
	 * connect to the file system
	 */
	pdir = proc_mkdir("profile", NULL);
	if (!pdir ||
	    !proc_create("data", 0, pdir, &profile_fops) ||
	    !proc_create("data1", 0, pdir, &profile_fops) ||
	    !proc_create("rate", 0, pdir, &profile_rate_fops)) {
		netap_profile_release_resource();
		return -ENOMEM;
	}

	profile_init(node[0]);

	/*
	 * attatch the device callback to N2H channel for CPU 0
	 */
	node[0]->ctx = nss_profiler_notify_register(NSS_CORE_0, profile_handle_nss_data, node[0]);
#if NSS_MAX_CORES > 1
	node[1] = node[0] + 1;
	node[1]->ccl = node[0]->ccl + CCL_SIZE;

	profile_init(node[1]);
	node[1]->ctx = nss_profiler_notify_register(NSS_CORE_1, profile_handle_nss_data, node[1]);
	profile_register_performance_counter(&node[1]->profile_sequence_num, "Profile1 DRV data packets");
#endif

	profile_register_performance_counter(&node[0]->profile_sequence_num, "Profile0 DRV data packets");
	return 0;
}

/*
 * kernel module exit
 */
void __exit netap_profile_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	nss_profiler_notify_unregister(NSS_CORE_0);
#if NSS_MAX_CORES > 1
	nss_profiler_notify_unregister(NSS_CORE_1);
#endif
	netap_profile_release_resource();
}

module_init(netap_profile_init_module);
module_exit(netap_profile_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
