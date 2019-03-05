/*
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2005 Atheros Communications, Inc.
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

#include "diag.h"

#include <getopt.h>

#include "ah.h"
#include "ah_devid.h"
#include "ah_internal.h"
#undef AH_PRIVATE		/* XXX twist defs in ar5212.h to suit */
#define	AH_PRIVATE(ah)	(ah)
#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"

static void printPcdacTable(FILE *fd, u_int16_t pcdac[], u_int n);
static void printPowerPerRate(FILE *fd, u_int16_t ratesArray[], u_int n);
static void printRevs(FILE *fd, const HAL_REVS *revs);

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s [-v] [-i dev]\n", progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif
	int s, i, verbose = 0, c;
	struct ath_diag atd;
	HAL_REVS revs;
	u_int16_t pcdacTable[MAX(PWR_TABLE_SIZE,PWR_TABLE_SIZE_2413)];
	u_int16_t ratesArray[16];
	u_int nrates, npcdac;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	strncpy(atd.ad_name, ATH_DEFAULT, sizeof (atd.ad_name));
	while ((c = getopt(argc, argv, "i:v")) != -1)
		switch (c) {
		case 'i':
			strncpy(atd.ad_name, optarg, sizeof (atd.ad_name));
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage(argv[0]);
		}

	atd.ad_id = HAL_DIAG_REVS;
	atd.ad_out_data = (caddr_t) &revs;
	atd.ad_out_size = sizeof(revs);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);

	if (verbose)
		printRevs(stdout, &revs);

	atd.ad_id = HAL_DIAG_TXRATES;
	atd.ad_out_data = (caddr_t) ratesArray;
	atd.ad_out_size = sizeof(ratesArray);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);
	nrates = sizeof(ratesArray) / sizeof(u_int16_t);

	atd.ad_id = HAL_DIAG_PCDAC;
	atd.ad_out_data = (caddr_t) pcdacTable;
	atd.ad_out_size = sizeof(pcdacTable);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);
	if (IS_2413(&revs))
		npcdac = PWR_TABLE_SIZE_2413;
	else
		npcdac = PWR_TABLE_SIZE;

	printf("PCDAC table:\n");
	printPcdacTable(stdout, pcdacTable, npcdac);

	printf("Power per rate table:\n");
	printPowerPerRate(stdout, ratesArray, nrates);

	return 0;
}

static void
printPcdacTable(FILE *fd, u_int16_t pcdac[], u_int n)
{
	int i, halfRates = n/2;

	for (i = 0; i < halfRates; i += 2)
		fprintf(fd, "[%2u] %04x %04x [%2u] %04x %04x\n",
			i, pcdac[2*i + 1], pcdac[2*i],
			i+1, pcdac[2*(i+1) + 1], pcdac[2*(i+1)]);
}

static void
printPowerPerRate(FILE *fd, u_int16_t ratesArray[], u_int n)
{
	const unsigned char *rateString[] = {
		" 6mb OFDM", " 9mb OFDM", "12mb OFDM", "18mb OFDM",
		"24mb OFDM", "36mb OFDM", "48mb OFDM", "54mb OFDM",
		"1L   CCK ", "2L   CCK ", "2S   CCK ", "5.5L CCK ",
		"5.5S CCK ", "11L  CCK ", "11S  CCK ", "XR       "
	};
	int i, halfRates = n/2;

	for (i = 0; i < halfRates; i++)
		fprintf(fd, " %s %3d.%1d dBm | %s %3d.%1d dBm\n", 
			 rateString[i], ratesArray[i]/2,
			 (ratesArray[i] %2) * 5, 
			 rateString[i + halfRates],
			 ratesArray[i + halfRates]/2,
			 (ratesArray[i + halfRates] %2) *5);
}

static void
printRevs(FILE *fd, const HAL_REVS *revs)
{
	const char *rfbackend;

	fprintf(fd, "PCI device id 0x%x subvendor id 0x%x\n",
		revs->ah_devid, revs->ah_subvendorid);
	fprintf(fd, "mac %d.%d phy %d.%d"
		, revs->ah_mac_version, revs->ah_mac_rev
		, revs->ah_phy_rev >> 4, revs->ah_phy_rev & 0xf
	);
	rfbackend = IS_2413(revs) ? "2413" : IS_5112(revs) ? "5112" : "5111";
	if (revs->ah_analog_5ghz_rev && revs->ah_analog2GhzRev)
		fprintf(fd, " 5ghz radio %d.%d 2ghz radio %d.%d (%s)\n"
			, revs->ah_analog_5ghz_rev >> 4
			, revs->ah_analog_5ghz_rev & 0xf
			, revs->ah_analog2GhzRev >> 4
			, revs->ah_analog2GhzRev & 0xf
			, rfbackend
		);
	else
		fprintf(fd, " radio %d.%d (%s)\n"
			, revs->ah_analog_5ghz_rev >> 4
			, revs->ah_analog_5ghz_rev & 0xf
			, rfbackend
		);
}
