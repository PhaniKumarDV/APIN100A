/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * Notifications and licenses are retained for attribution purposes only.
 */
/*
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2005 Atheros Communications, Inc.
 * Copyright (c) 2010, Atheros Communications Inc. 
 * 
 * Redistribution and use in source and binary forms are permitted
 * provided that the following conditions are met:
 * 1. The materials contained herein are unmodified and are used
 *    unmodified.
 * 2. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following NO
 *    ''WARRANTY'' disclaimer below (''Disclaimer''), without
 *    modification.
 * 3. Redistributions in binary form must reproduce at minimum a
 *    disclaimer similar to the Disclaimer below and any redistribution
 *    must be conditioned upon including a substantially similar
 *    Disclaimer requirement for further binary redistribution.
 * 4. Neither the names of the above-listed copyright holders nor the
 *    names of any contributors may be used to endorse or promote
 *    product derived from this software without specific prior written
 *    permission.
 * 
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT,
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 * FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES.
 */

#include "opt_ah.h"

#include "ah.h"
#include "ah_internal.h"
#include "ah_eeprom.h"

static void
getPcdacInterceptsFromPcdacMinMax(HAL_EEPROM *ee,
	u_int16_t pcdacMin, u_int16_t pcdacMax, u_int16_t *vp)
{
	static const u_int16_t intercepts3[] =
		{ 0, 5, 10, 20, 30, 50, 70, 85, 90, 95, 100 };
	static const u_int16_t intercepts3_2[] =
		{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	const u_int16_t *ip = ee->ee_version < AR_EEPROM_VER3_2 ?
		intercepts3 : intercepts3_2;
	int i;

	/* loop for the percentages in steps or 5 */
	for (i = 0; i < NUM_INTERCEPTS; i++ )
		*vp++ = (ip[i] * pcdacMax + (100 - ip[i]) * pcdacMin) / 100;
}

/*
 * Get channel value from binary representation held in eeprom
 */
static u_int16_t
fbin2freq(HAL_EEPROM *ee, u_int16_t fbin)
{
	if (fbin == CHANNEL_UNUSED)	/* reserved value, don't convert */
		return fbin;
	return ee->ee_version <= AR_EEPROM_VER3_2 ?
		(fbin > 62 ? 5100 + 10*62 + 5*(fbin-62) : 5100 + 10*fbin) :
		4800 + 5*fbin;
}

static u_int16_t
fbin2freq_2p4(HAL_EEPROM *ee, u_int16_t fbin)
{
	if (fbin == CHANNEL_UNUSED)	/* reserved value, don't convert */
		return fbin;
	return ee->ee_version <= AR_EEPROM_VER3_2 ?
		2400 + fbin :
		2300 + fbin;
}

/*
 * Now copy EEPROM frequency pier contents into the allocated space
 */
static bool
readEepromFreqPierInfo(struct ath_hal *ah, HAL_EEPROM *ee)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	u_int16_t eeval, off;
	int i;

	if ((ee->ee_numChannels11a < 0)||(ee->ee_numChannels11a > NUM_11A_EEPROM_CHANNELS)) {
		return false;
	}

	if (ee->ee_version >= AR_EEPROM_VER4_0 &&
	    ee->ee_eepMap && !ee->ee_Amode) {
		/*
		 * V4.0 EEPROMs with map type 1 have frequency pier
		 * data only when 11a mode is supported.
		 */
		return true;
	}
	if (ee->ee_version >= AR_EEPROM_VER3_3) {
		off = GROUPS_OFFSET3_3 + GROUP1_OFFSET;
		for (i = 0; i < ee->ee_numChannels11a; i += 2) {
			EEREAD(off++);
			ee->ee_channels11a[i]   = (eeval >> 8) & FREQ_MASK_3_3;
			ee->ee_channels11a[i+1] = eeval & FREQ_MASK_3_3;
		} 
	} else {
		off = GROUPS_OFFSET3_2 + GROUP1_OFFSET;

		EEREAD(off++);
		ee->ee_channels11a[0] = (eeval >> 9) & FREQ_MASK;
		ee->ee_channels11a[1] = (eeval >> 2) & FREQ_MASK;
		ee->ee_channels11a[2] = (eeval << 5) & FREQ_MASK;

		EEREAD(off++);
		ee->ee_channels11a[2] |= (eeval >> 11) & 0x1f;
		ee->ee_channels11a[3]  = (eeval >>  4) & FREQ_MASK;
		ee->ee_channels11a[4]  = (eeval <<  3) & FREQ_MASK;

		EEREAD(off++);
		ee->ee_channels11a[4] |= (eeval >> 13) & 0x7;
		ee->ee_channels11a[5]  = (eeval >>  6) & FREQ_MASK;
		ee->ee_channels11a[6]  = (eeval <<  1) & FREQ_MASK;

		EEREAD(off++);
		ee->ee_channels11a[6] |= (eeval >> 15) & 0x1;
		ee->ee_channels11a[7]  = (eeval >>  8) & FREQ_MASK;
		ee->ee_channels11a[8]  = (eeval >>  1) & FREQ_MASK;
		ee->ee_channels11a[9]  = (eeval <<  6) & FREQ_MASK;

		EEREAD(off++);
		ee->ee_channels11a[9] |= (eeval >> 10) & 0x3f;
	}

	for (i = 0; i < ee->ee_numChannels11a; i++)
		ee->ee_channels11a[i] = fbin2freq(ee, ee->ee_channels11a[i]);

	return true;
#undef EEREAD
}

/*
 * Rev 4 Eeprom 5112 Power Extract Functions
 */

/*
 * Allocate the power information based on the number of channels
 * recorded by the calibration.  These values are then initialized.
 */
static bool
eepromAllocExpnPower5112(struct ath_hal *ah,
	EEPROM_POWER_5112 *pCalDataset,
	EEPROM_POWER_EXPN_5112 *pPowerExpn)
{
	u_int16_t num_channels = pCalDataset->num_channels;
	const u_int16_t *pChanList = pCalDataset->p_channels;
	void *data;
	int i, j;

	/* Allocate the channel and Power Data arrays together */
	data = ath_hal_malloc(ah, 
		roundup(sizeof(u_int16_t) * num_channels, sizeof(u_int32_t)) +
		sizeof(EXPN_DATA_PER_CHANNEL_5112) * num_channels);
	if (data == AH_NULL) {
		HDPRINTF(ah, HAL_DBG_EEPROM, "%s unable to allocate raw data struct (gen3)\n",
			__func__);
		return false;
	}
	pPowerExpn->p_channels = data;
	pPowerExpn->pDataPerChannel = (void *)(((char *)data) +
		roundup(sizeof(u_int16_t) * num_channels, sizeof(u_int32_t)));

	pPowerExpn->num_channels = num_channels;
	for (i = 0; i < num_channels; i++) {
		pPowerExpn->p_channels[i] =
			pPowerExpn->pDataPerChannel[i].channelValue =
				pChanList[i];
		for (j = 0; j < NUM_XPD_PER_CHANNEL; j++) {
			pPowerExpn->pDataPerChannel[i].pDataPerXPD[j].xpd_gain = j;
			pPowerExpn->pDataPerChannel[i].pDataPerXPD[j].numPcdacs = 0;
		}
		pPowerExpn->pDataPerChannel[i].pDataPerXPD[0].numPcdacs = 4;
		pPowerExpn->pDataPerChannel[i].pDataPerXPD[3].numPcdacs = 3;
	}
	return true;
}

/*
 * Expand the dataSet from the calibration information into the
 * final power structure for 5112
 */
static bool
eepromExpandPower5112(struct ath_hal *ah,
	const EEPROM_POWER_5112 *pCalDataset,
	EEPROM_POWER_EXPN_5112 *pPowerExpn)
{
	int ii, jj, kk;
	EXPN_DATA_PER_XPD_5112 *pExpnXPD;
	/* ptr to array of info held per channel */
	const EEPROM_DATA_PER_CHANNEL_5112 *pCalCh;
	u_int16_t xgainList[2], xpdMask;

	pPowerExpn->xpdMask = pCalDataset->xpdMask;

	xgainList[0] = 0xDEAD;
	xgainList[1] = 0xDEAD;

	kk = 0;
	xpdMask = pPowerExpn->xpdMask;
	for (jj = 0; jj < NUM_XPD_PER_CHANNEL; jj++) {
		if (((xpdMask >> jj) & 1) > 0) {
			if (kk > 1) {
				HDPRINTF(ah, HAL_DBG_EEPROM,
                         "%s: too many xpdGains in dataset: %u\n",
                         __func__, kk);
				return false;
			}
			xgainList[kk++] = jj;
		}
	}

	pPowerExpn->num_channels = pCalDataset->num_channels;
	if (pPowerExpn->num_channels == 0) {
		HDPRINTF(ah, HAL_DBG_EEPROM, "%s: no channels\n", __func__);
		return false;
	}

	for (ii = 0; ii < pPowerExpn->num_channels; ii++) {
		pCalCh = &pCalDataset->pDataPerChannel[ii];
		pPowerExpn->pDataPerChannel[ii].channelValue =
			pCalCh->channelValue;
		pPowerExpn->pDataPerChannel[ii].maxPower_t4 =
			pCalCh->maxPower_t4;

		for (jj = 0; jj < NUM_XPD_PER_CHANNEL; jj++)
			pPowerExpn->pDataPerChannel[ii].pDataPerXPD[jj].numPcdacs = 0;
		if (xgainList[1] == 0xDEAD) {
			jj = xgainList[0];
			pExpnXPD = &pPowerExpn->pDataPerChannel[ii].pDataPerXPD[jj];
			pExpnXPD->numPcdacs = 4;
			pExpnXPD->pcdac[0] = pCalCh->pcd1_xg0;
			pExpnXPD->pcdac[1] = (u_int16_t)
				(pExpnXPD->pcdac[0] + pCalCh->pcd2_delta_xg0);
			pExpnXPD->pcdac[2] = (u_int16_t)
				(pExpnXPD->pcdac[1] + pCalCh->pcd3_delta_xg0);
			pExpnXPD->pcdac[3] = (u_int16_t)
				(pExpnXPD->pcdac[2] + pCalCh->pcd4_delta_xg0);

			pExpnXPD->pwr_t4[0] = pCalCh->pwr1_xg0;
			pExpnXPD->pwr_t4[1] = pCalCh->pwr2_xg0;
			pExpnXPD->pwr_t4[2] = pCalCh->pwr3_xg0;
			pExpnXPD->pwr_t4[3] = pCalCh->pwr4_xg0;

		} else {
			pPowerExpn->pDataPerChannel[ii].pDataPerXPD[xgainList[0]].pcdac[0] = pCalCh->pcd1_xg0;
			pPowerExpn->pDataPerChannel[ii].pDataPerXPD[xgainList[1]].pcdac[0] = 20;
			pPowerExpn->pDataPerChannel[ii].pDataPerXPD[xgainList[1]].pcdac[1] = 35;
			pPowerExpn->pDataPerChannel[ii].pDataPerXPD[xgainList[1]].pcdac[2] = 63;

			jj = xgainList[0];
			pExpnXPD = &pPowerExpn->pDataPerChannel[ii].pDataPerXPD[jj];
			pExpnXPD->numPcdacs = 4;
			pExpnXPD->pcdac[1] = (u_int16_t)
				(pExpnXPD->pcdac[0] + pCalCh->pcd2_delta_xg0);
			pExpnXPD->pcdac[2] = (u_int16_t)
				(pExpnXPD->pcdac[1] + pCalCh->pcd3_delta_xg0);
			pExpnXPD->pcdac[3] = (u_int16_t)
				(pExpnXPD->pcdac[2] + pCalCh->pcd4_delta_xg0);
			pExpnXPD->pwr_t4[0] = pCalCh->pwr1_xg0;
			pExpnXPD->pwr_t4[1] = pCalCh->pwr2_xg0;
			pExpnXPD->pwr_t4[2] = pCalCh->pwr3_xg0;
			pExpnXPD->pwr_t4[3] = pCalCh->pwr4_xg0;

			jj = xgainList[1];
			pExpnXPD = &pPowerExpn->pDataPerChannel[ii].pDataPerXPD[jj];
			pExpnXPD->numPcdacs = 3;

			pExpnXPD->pwr_t4[0] = pCalCh->pwr1_xg3;
			pExpnXPD->pwr_t4[1] = pCalCh->pwr2_xg3;
			pExpnXPD->pwr_t4[2] = pCalCh->pwr3_xg3;
		}
	}
	return true;
}

static bool
readEepromRawPowerCalInfo5112(struct ath_hal *ah, HAL_EEPROM *ee)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	const u_int16_t dbmmask		 = 0xff;
	const u_int16_t pcdac_delta_mask = 0x1f;
	const u_int16_t pcdac_mask	 = 0x3f;
	const u_int16_t freqmask	 = 0xff;

	int i, mode, num_piers;
	u_int32_t off;
	u_int16_t eeval;
	u_int16_t freq[NUM_11A_EEPROM_CHANNELS];
        EEPROM_POWER_5112 eePower;

	HALASSERT(ee->ee_version >= AR_EEPROM_VER4_0);
	off = GROUPS_OFFSET3_3;
	for (mode = headerInfo11A; mode <= headerInfo11G; mode++) {
		num_piers = 0;
		switch (mode) {
		case headerInfo11A:
			if (!ee->ee_Amode)	/* no 11a calibration data */
				continue;
			while (num_piers < NUM_11A_EEPROM_CHANNELS) {
				EEREAD(off++);
				if ((eeval & freqmask) == 0)
					break;
				freq[num_piers++] = fbin2freq(ee,
					eeval & freqmask);

				if (((eeval >> 8) & freqmask) == 0)
					break;
				freq[num_piers++] = fbin2freq(ee,
					(eeval>>8) & freqmask);
			}
			break;
		case headerInfo11B:
			if (!ee->ee_Bmode)	/* no 11b calibration data */
				continue;
			for (i = 0; i < NUM_2_4_EEPROM_CHANNELS; i++)
				if (ee->ee_calPier11b[i] != CHANNEL_UNUSED)
					freq[num_piers++] = ee->ee_calPier11b[i];
			break;
		case headerInfo11G:
			if (!ee->ee_Gmode)	/* no 11g calibration data */
				continue;
			for (i = 0; i < NUM_2_4_EEPROM_CHANNELS; i++)
				if (ee->ee_calPier11g[i] != CHANNEL_UNUSED)
					freq[num_piers++] = ee->ee_calPier11g[i];
			break;
		default:
			HDPRINTF(ah, HAL_DBG_EEPROM, "%s: invalid mode 0x%x\n", __func__, mode);
			return false;
		}

		OS_MEMZERO(&eePower, sizeof(eePower));
		eePower.num_channels = num_piers;

		for (i = 0; i < num_piers; i++) {
			eePower.p_channels[i] = freq[i];
			eePower.pDataPerChannel[i].channelValue = freq[i];

			EEREAD(off++);
			eePower.pDataPerChannel[i].pwr1_xg0 = (int16_t)
				((eeval & dbmmask) - ((eeval >> 7) & 0x1)*256);
			eePower.pDataPerChannel[i].pwr2_xg0 = (int16_t)
				(((eeval >> 8) & dbmmask) - ((eeval >> 15) & 0x1)*256);

			EEREAD(off++);
			eePower.pDataPerChannel[i].pwr3_xg0 = (int16_t)
				((eeval & dbmmask) - ((eeval >> 7) & 0x1)*256);
			eePower.pDataPerChannel[i].pwr4_xg0 = (int16_t)
				(((eeval >> 8) & dbmmask) - ((eeval >> 15) & 0x1)*256);

			EEREAD(off++);
			eePower.pDataPerChannel[i].pcd2_delta_xg0 = (u_int16_t)
				(eeval & pcdac_delta_mask);
			eePower.pDataPerChannel[i].pcd3_delta_xg0 = (u_int16_t)
				((eeval >> 5) & pcdac_delta_mask);
			eePower.pDataPerChannel[i].pcd4_delta_xg0 = (u_int16_t)
				((eeval >> 10) & pcdac_delta_mask);

			EEREAD(off++);
			eePower.pDataPerChannel[i].pwr1_xg3 = (int16_t)
				((eeval & dbmmask) - ((eeval >> 7) & 0x1)*256);
			eePower.pDataPerChannel[i].pwr2_xg3 = (int16_t)
				(((eeval >> 8) & dbmmask) - ((eeval >> 15) & 0x1)*256);

			EEREAD(off++);
			eePower.pDataPerChannel[i].pwr3_xg3 = (int16_t)
				((eeval & dbmmask) - ((eeval >> 7) & 0x1)*256);
			if (ee->ee_version >= AR_EEPROM_VER4_3) {
				eePower.pDataPerChannel[i].maxPower_t4 =
					eePower.pDataPerChannel[i].pwr4_xg0;     
				eePower.pDataPerChannel[i].pcd1_xg0 = (u_int16_t)
					((eeval >> 8) & pcdac_mask);
			} else {
				eePower.pDataPerChannel[i].maxPower_t4 = (int16_t)
					(((eeval >> 8) & dbmmask) -
					 ((eeval >> 15) & 0x1)*256);
				eePower.pDataPerChannel[i].pcd1_xg0 = 1;
			}
		}
		eePower.xpdMask = ee->ee_xgain[mode];

		if (!eepromAllocExpnPower5112(ah, &eePower, &ee->ee_modePowerArray5112[mode])) {
			HDPRINTF(ah, HAL_DBG_EEPROM, "%s: did not allocate power struct\n",
				__func__);
			return false;
                }
                if (!eepromExpandPower5112(ah, &eePower, &ee->ee_modePowerArray5112[mode])) {
			HDPRINTF(ah, HAL_DBG_EEPROM, "%s: did not expand power struct\n",
				__func__);
			return false;
		}
	}
	return true;
#undef EEREAD
}

   static void
freeEepromRawPowerCalInfo5112(struct ath_hal *ah, HAL_EEPROM *ee)
{
	int mode;
	void *data;

	for (mode = headerInfo11A; mode <= headerInfo11G; mode++) {
		EEPROM_POWER_EXPN_5112 *pPowerExpn =
			&ee->ee_modePowerArray5112[mode];
		data = pPowerExpn->p_channels;
		if (data != AH_NULL) {
			pPowerExpn->p_channels = AH_NULL;
			ath_hal_free(ah, data);
		}
	}
}


static void
ar2413SetupEEPROMDataset(EEPROM_DATA_STRUCT_2413 *pEEPROMDataset2413,
	u_int16_t myNumRawChannels, u_int16_t *pMyRawChanList)
{
	u_int16_t i, channelValue;
	u_int32_t xpd_mask;
	u_int16_t numPdGainsUsed;

	pEEPROMDataset2413->num_channels = myNumRawChannels;

	xpd_mask = pEEPROMDataset2413->xpd_mask;
	numPdGainsUsed = 0;
	if ((xpd_mask >> 0) & 0x1) numPdGainsUsed++;
	if ((xpd_mask >> 1) & 0x1) numPdGainsUsed++;
	if ((xpd_mask >> 2) & 0x1) numPdGainsUsed++;
	if ((xpd_mask >> 3) & 0x1) numPdGainsUsed++;

	for (i = 0; i < myNumRawChannels; i++) {
		channelValue = pMyRawChanList[i];
		pEEPROMDataset2413->p_channels[i] = channelValue;
		pEEPROMDataset2413->pDataPerChannel[i].channelValue = channelValue;
		pEEPROMDataset2413->pDataPerChannel[i].numPdGains = numPdGainsUsed;
	}
}

static bool
ar2413ReadCalDataset(struct ath_hal *ah, HAL_EEPROM *ee,
	EEPROM_DATA_STRUCT_2413 *pCalDataset,
	u_int32_t start_offset, u_int32_t maxPiers, u_int8_t mode)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	const u_int16_t dbm_I_mask = 0x1F;	/* 5-bits. 1dB step. */
	const u_int16_t dbm_delta_mask = 0xF;	/* 4-bits. 0.5dB step. */
	const u_int16_t Vpd_I_mask = 0x7F;	/* 7-bits. 0-128 */
	const u_int16_t Vpd_delta_mask = 0x3F;	/* 6-bits. 0-63 */
	const u_int16_t freqmask = 0xff;

	u_int16_t ii, eeval=0;
	u_int16_t idx, num_piers;
	u_int16_t freq[NUM_11A_EEPROM_CHANNELS] =
            {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	if ((maxPiers < 0)||(maxPiers > NUM_11A_EEPROM_CHANNELS)) {
		return false;
	}

	idx = start_offset;
	for (num_piers = 0; num_piers < maxPiers;) {
		EEREAD(idx++);
		if ((eeval & freqmask) == 0)
			break;
		if (mode == headerInfo11A)
			freq[num_piers++] = fbin2freq(ee, (eeval & freqmask));
		else
			freq[num_piers++] = fbin2freq_2p4(ee, (eeval & freqmask));
		if (((eeval >> 8) & freqmask) == 0)
			break;
		if (mode == headerInfo11A)
			freq[num_piers++] = fbin2freq(ee, (eeval >> 8) & freqmask);
		else
			freq[num_piers++] = fbin2freq_2p4(ee, (eeval >> 8) & freqmask);
	}
	ar2413SetupEEPROMDataset(pCalDataset, num_piers, &freq[0]);

	idx = start_offset + (maxPiers / 2);
	for (ii = 0; ii < pCalDataset->num_channels; ii++) {
		EEPROM_DATA_PER_CHANNEL_2413 *currCh =
			&(pCalDataset->pDataPerChannel[ii]);

		if (currCh->numPdGains > 0) {
			/*
			 * Read the first NUM_POINTS_OTHER_PDGAINS pwr
			 * and Vpd values for pdgain_0
			 */
			EEREAD(idx++);
			currCh->pwr_I[0] = eeval & dbm_I_mask;
			currCh->Vpd_I[0] = (eeval >> 5) & Vpd_I_mask;
			currCh->pwr_delta_t2[0][0] =
				(eeval >> 12) & dbm_delta_mask;
			
			EEREAD(idx++);
			currCh->Vpd_delta[0][0] = eeval & Vpd_delta_mask;
			currCh->pwr_delta_t2[1][0] =
				(eeval >> 6) & dbm_delta_mask;
			currCh->Vpd_delta[1][0] =
				(eeval >> 10) & Vpd_delta_mask;
			
			EEREAD(idx++);
			currCh->pwr_delta_t2[2][0] = eeval & dbm_delta_mask;
			currCh->Vpd_delta[2][0] = (eeval >> 4) & Vpd_delta_mask;
		}
		
		if (currCh->numPdGains > 1) {
			/*
			 * Read the first NUM_POINTS_OTHER_PDGAINS pwr
			 * and Vpd values for pdgain_1
			 */
			currCh->pwr_I[1] = (eeval >> 10) & dbm_I_mask;
			currCh->Vpd_I[1] = (eeval >> 15) & 0x1;
			
			EEREAD(idx++);
			/* upper 6 bits */
			currCh->Vpd_I[1] |= (eeval & 0x3F) << 1;
			currCh->pwr_delta_t2[0][1] =
				(eeval >> 6) & dbm_delta_mask;
			currCh->Vpd_delta[0][1] =
				(eeval >> 10) & Vpd_delta_mask;
			
			EEREAD(idx++);
			currCh->pwr_delta_t2[1][1] = eeval & dbm_delta_mask;
			currCh->Vpd_delta[1][1] = (eeval >> 4) & Vpd_delta_mask;
			currCh->pwr_delta_t2[2][1] =
				(eeval >> 10) & dbm_delta_mask;
			currCh->Vpd_delta[2][1] = (eeval >> 14) & 0x3;
			
			EEREAD(idx++);
			/* upper 4 bits */
			currCh->Vpd_delta[2][1] |= (eeval & 0xF) << 2;
		} else if (currCh->numPdGains == 1) {
			/*
			 * Read the last pwr and Vpd values for pdgain_0
			 */
			currCh->pwr_delta_t2[3][0] =
				(eeval >> 10) & dbm_delta_mask;
			currCh->Vpd_delta[3][0] = (eeval >> 14) & 0x3;

			EEREAD(idx++);
			/* upper 4 bits */
			currCh->Vpd_delta[3][0] |= (eeval & 0xF) << 2;

			/* 4 words if numPdGains == 1 */
		}

		if (currCh->numPdGains > 2) {
			/*
			 * Read the first NUM_POINTS_OTHER_PDGAINS pwr
			 * and Vpd values for pdgain_2
			 */
			currCh->pwr_I[2] = (eeval >> 4) & dbm_I_mask;
			currCh->Vpd_I[2] = (eeval >> 9) & Vpd_I_mask;
			
			EEREAD(idx++);
			currCh->pwr_delta_t2[0][2] =
				(eeval >> 0) & dbm_delta_mask;
			currCh->Vpd_delta[0][2] = (eeval >> 4) & Vpd_delta_mask;
			currCh->pwr_delta_t2[1][2] =
				(eeval >> 10) & dbm_delta_mask;
			currCh->Vpd_delta[1][2] = (eeval >> 14) & 0x3;
			
			EEREAD(idx++);
			/* upper 4 bits */
			currCh->Vpd_delta[1][2] |= (eeval & 0xF) << 2;
			currCh->pwr_delta_t2[2][2] =
				(eeval >> 4) & dbm_delta_mask;
			currCh->Vpd_delta[2][2] = (eeval >> 8) & Vpd_delta_mask;
		} else if (currCh->numPdGains == 2) {
			/*
			 * Read the last pwr and Vpd values for pdgain_1
			 */
			currCh->pwr_delta_t2[3][1] =
				(eeval >> 4) & dbm_delta_mask;
			currCh->Vpd_delta[3][1] = (eeval >> 8) & Vpd_delta_mask;

			/* 6 words if numPdGains == 2 */
		}

		if (currCh->numPdGains > 3) {
			/*
			 * Read the first NUM_POINTS_OTHER_PDGAINS pwr
			 * and Vpd values for pdgain_3
			 */
			currCh->pwr_I[3] = (eeval >> 14) & 0x3;
			
			EEREAD(idx++);
			/* upper 3 bits */
			currCh->pwr_I[3] |= ((eeval >> 0) & 0x7) << 2;
			currCh->Vpd_I[3] = (eeval >> 3) & Vpd_I_mask;
			currCh->pwr_delta_t2[0][3] =
				(eeval >> 10) & dbm_delta_mask;
			currCh->Vpd_delta[0][3] = (eeval >> 14) & 0x3;
			
			EEREAD(idx++);
			/* upper 4 bits */
			currCh->Vpd_delta[0][3] |= (eeval & 0xF) << 2;
			currCh->pwr_delta_t2[1][3] =
				(eeval >> 4) & dbm_delta_mask;
			currCh->Vpd_delta[1][3] = (eeval >> 8) & Vpd_delta_mask;
			currCh->pwr_delta_t2[2][3] = (eeval >> 14) & 0x3;
			
			EEREAD(idx++);
			/* upper 2 bits */
			currCh->pwr_delta_t2[2][3] |= ((eeval >> 0) & 0x3) << 2;
			currCh->Vpd_delta[2][3] = (eeval >> 2) & Vpd_delta_mask;
			currCh->pwr_delta_t2[3][3] =
				(eeval >> 8) & dbm_delta_mask;
			currCh->Vpd_delta[3][3] = (eeval >> 12) & 0xF;
			
			EEREAD(idx++);
			/* upper 2 bits */
			currCh->Vpd_delta[3][3] |= ((eeval >> 0) & 0x3) << 4;

			/* 12 words if numPdGains == 4 */
		} else if (currCh->numPdGains == 3) {
			/* read the last pwr and Vpd values for pdgain_2 */
			currCh->pwr_delta_t2[3][2] = (eeval >> 14) & 0x3;
			
			EEREAD(idx++);
			/* upper 2 bits */
			currCh->pwr_delta_t2[3][2] |= ((eeval >> 0) & 0x3) << 2;
			currCh->Vpd_delta[3][2] = (eeval >> 2) & Vpd_delta_mask;

			/* 9 words if numPdGains == 3 */
		}
	}
	return true;
#undef EEREAD
}

static void
ar2413SetupRawDataset(RAW_DATA_STRUCT_2413 *pRaw, EEPROM_DATA_STRUCT_2413 *pCal)
{
	u_int16_t i, j, kk, channelValue;
	u_int16_t xpd_mask;
	u_int16_t numPdGainsUsed;

	pRaw->num_channels = pCal->num_channels;

	xpd_mask = pRaw->xpd_mask;
	numPdGainsUsed = 0;
	if ((xpd_mask >> 0) & 0x1) numPdGainsUsed++;
	if ((xpd_mask >> 1) & 0x1) numPdGainsUsed++;
	if ((xpd_mask >> 2) & 0x1) numPdGainsUsed++;
	if ((xpd_mask >> 3) & 0x1) numPdGainsUsed++;

	for (i = 0; i < pCal->num_channels; i++) {
		channelValue = pCal->p_channels[i];

		pRaw->p_channels[i] = channelValue;

		pRaw->pDataPerChannel[i].channelValue = channelValue;
		pRaw->pDataPerChannel[i].numPdGains = numPdGainsUsed;

		kk = 0;
		for (j = 0; j < MAX_NUM_PDGAINS_PER_CHANNEL; j++) {
			pRaw->pDataPerChannel[i].pDataPerPDGain[j].pd_gain = j;
			if ((xpd_mask >> j) & 0x1) {
				pRaw->pDataPerChannel[i].pDataPerPDGain[j].numVpd = NUM_POINTS_OTHER_PDGAINS;
				kk++;
				if (kk == 1) {
					/* 
					 * lowest pd_gain corresponds
					 *  to highest power and thus,
					 *  has one more point
					 */
					pRaw->pDataPerChannel[i].pDataPerPDGain[j].numVpd = NUM_POINTS_LAST_PDGAIN;
				}
			} else {
				pRaw->pDataPerChannel[i].pDataPerPDGain[j].numVpd = 0;
			}
		}
	}
}

static bool
ar2413EepromToRawDataset(struct ath_hal *ah,
	EEPROM_DATA_STRUCT_2413 *pCal, RAW_DATA_STRUCT_2413 *pRaw)
{
	u_int16_t ii, jj, kk, ss;
	RAW_DATA_PER_PDGAIN_2413 *pRawXPD;
	/* ptr to array of info held per channel */
	EEPROM_DATA_PER_CHANNEL_2413 *pCalCh;
	u_int16_t xgain_list[MAX_NUM_PDGAINS_PER_CHANNEL];
	u_int16_t xpd_mask;
	u_int32_t numPdGainsUsed;

	HALASSERT(pRaw->xpd_mask == pCal->xpd_mask);

	xgain_list[0] = 0xDEAD;
	xgain_list[1] = 0xDEAD;
	xgain_list[2] = 0xDEAD;
	xgain_list[3] = 0xDEAD;

	numPdGainsUsed = 0;
	xpd_mask = pRaw->xpd_mask;
	for (jj = 0; jj < MAX_NUM_PDGAINS_PER_CHANNEL; jj++) {
		if ((xpd_mask >> (MAX_NUM_PDGAINS_PER_CHANNEL-jj-1)) & 1)
			xgain_list[numPdGainsUsed++] = MAX_NUM_PDGAINS_PER_CHANNEL-jj-1;
	}

	pRaw->num_channels = pCal->num_channels;
	for (ii = 0; ii < pRaw->num_channels; ii++) {
		pCalCh = &(pCal->pDataPerChannel[ii]);
		pRaw->pDataPerChannel[ii].channelValue = pCalCh->channelValue;

		/* numVpd has already been setup appropriately for the relevant pdGains */
		for (jj = 0; jj < numPdGainsUsed; jj++) {
			/* use jj for calDataset and ss for rawDataset */
			ss = xgain_list[jj];
			pRawXPD = &(pRaw->pDataPerChannel[ii].pDataPerPDGain[ss]);
			HALASSERT(pRawXPD->numVpd >= 1);

			pRawXPD->pwr_t4[0] = (u_int16_t)(4*pCalCh->pwr_I[jj]);
			pRawXPD->Vpd[0]    = pCalCh->Vpd_I[jj];

			for (kk = 1; kk < pRawXPD->numVpd; kk++) {
				pRawXPD->pwr_t4[kk] = (int16_t)(pRawXPD->pwr_t4[kk-1] + 2*pCalCh->pwr_delta_t2[kk-1][jj]);
				pRawXPD->Vpd[kk] = (u_int16_t)(pRawXPD->Vpd[kk-1] + pCalCh->Vpd_delta[kk-1][jj]);
			}
			/* loop over Vpds */
		}
		/* loop over pd_gains */
	}
	/* loop over channels */
	return true;
}

static bool
readEepromRawPowerCalInfo2413(struct ath_hal *ah, HAL_EEPROM *ee)
{
	/* NB: index is 1 less than numPdgains */
	const u_int16_t wordsForPdgains[] = { 4, 6, 9, 12 };
	EEPROM_DATA_STRUCT_2413 *pCal = AH_NULL;
	RAW_DATA_STRUCT_2413 *pRaw;
	int numEEPROMWordsPerChannel;
	u_int32_t off;
	bool ret = false;
	
	HALASSERT(ee->ee_version >= AR_EEPROM_VER5_0);
	HALASSERT(ee->ee_eepMap == 2);
	
	pCal = ath_hal_malloc(ah, sizeof(EEPROM_DATA_STRUCT_2413));
	if (pCal == AH_NULL)
		goto exit;
	
	off = ee->ee_eepMap2PowerCalStart;
	if (ee->ee_Amode) {
		OS_MEMZERO(pCal, sizeof(EEPROM_DATA_STRUCT_2413));
		pCal->xpd_mask = ee->ee_xgain[headerInfo11A];
		if (!ar2413ReadCalDataset(ah, ee, pCal, off,
			NUM_11A_EEPROM_CHANNELS_2413, headerInfo11A)) {
			goto exit;
		}
		pRaw = &ee->ee_rawDataset2413[headerInfo11A];
		pRaw->xpd_mask = ee->ee_xgain[headerInfo11A];
		ar2413SetupRawDataset(pRaw, pCal);
		if (!ar2413EepromToRawDataset(ah, pCal, pRaw)) {
			goto exit;
		}
		/* setup offsets for mode_11a next */
		numEEPROMWordsPerChannel = wordsForPdgains[
			pCal->pDataPerChannel[0].numPdGains - 1];
		off += pCal->num_channels * numEEPROMWordsPerChannel + 5;
	}
	if (ee->ee_Bmode) {
		OS_MEMZERO(pCal, sizeof(EEPROM_DATA_STRUCT_2413));
		pCal->xpd_mask = ee->ee_xgain[headerInfo11B];
		if (!ar2413ReadCalDataset(ah, ee, pCal, off,
			NUM_2_4_EEPROM_CHANNELS_2413 , headerInfo11B)) {
			goto exit;
		}
		pRaw = &ee->ee_rawDataset2413[headerInfo11B];
		pRaw->xpd_mask = ee->ee_xgain[headerInfo11B];
		ar2413SetupRawDataset(pRaw, pCal);
		if (!ar2413EepromToRawDataset(ah, pCal, pRaw)) {
			goto exit;
		}
		/* setup offsets for mode_11g next */
		numEEPROMWordsPerChannel = wordsForPdgains[
			pCal->pDataPerChannel[0].numPdGains - 1];
		off += pCal->num_channels * numEEPROMWordsPerChannel + 2;
	}
	if (ee->ee_Gmode) {
		OS_MEMZERO(pCal, sizeof(EEPROM_DATA_STRUCT_2413));
		pCal->xpd_mask = ee->ee_xgain[headerInfo11G];
		if (!ar2413ReadCalDataset(ah, ee, pCal, off,
			NUM_2_4_EEPROM_CHANNELS_2413, headerInfo11G)) {
			goto exit;
		}
		pRaw = &ee->ee_rawDataset2413[headerInfo11G];
		pRaw->xpd_mask = ee->ee_xgain[headerInfo11G];
		ar2413SetupRawDataset(pRaw, pCal);
		if (!ar2413EepromToRawDataset(ah, pCal, pRaw)) {
			goto exit;
		}
	}
	ret = true;
 exit:
	if (pCal != AH_NULL)
		ath_hal_free(ah, pCal);
	return ret;
}

/*
 * Now copy EEPROM Raw Power Calibration per frequency contents 
 * into the allocated space
 */
static bool
readEepromRawPowerCalInfo(struct ath_hal *ah, HAL_EEPROM *ee)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	u_int16_t eeval, nchan;
	u_int32_t off;
	int i, j, mode;

        if (ee->ee_version >= AR_EEPROM_VER4_0 && ee->ee_eepMap == 1)
		return readEepromRawPowerCalInfo5112(ah, ee);
	if (ee->ee_version >= AR_EEPROM_VER5_0 && ee->ee_eepMap == 2)
		return readEepromRawPowerCalInfo2413(ah, ee);

	/*
	 * Group 2:  read raw power data for all frequency piers
	 *
	 * NOTE: Group 2 contains the raw power calibration
	 *	 information for each of the channels that
	 *	 we recorded above.
	 */
	for (mode = headerInfo11A; mode <= headerInfo11G; mode++) {
		u_int16_t *p_channels = AH_NULL;
		DATA_PER_CHANNEL *pChannelData = AH_NULL;

		off = ee->ee_version >= AR_EEPROM_VER3_3 ? 
			GROUPS_OFFSET3_3 : GROUPS_OFFSET3_2;
		switch (mode) {
		case headerInfo11A:
			off      	+= GROUP2_OFFSET;
			nchan		= ee->ee_numChannels11a;
			pChannelData	= ee->ee_dataPerChannel11a;
			p_channels	= ee->ee_channels11a;
			break;
		case headerInfo11B:
			if (!ee->ee_Bmode)
				continue;
			off		+= GROUP3_OFFSET;
			nchan		= ee->ee_numChannels2_4;
			pChannelData	= ee->ee_dataPerChannel11b;
			p_channels	= ee->ee_channels11b;
			break;
		case headerInfo11G:
			if (!ee->ee_Gmode)
				continue;
			off		+= GROUP4_OFFSET;
			nchan		= ee->ee_numChannels2_4;
			pChannelData	= ee->ee_dataPerChannel11g;
			p_channels	= ee->ee_channels11g;
			break;
		default:
			HDPRINTF(ah, HAL_DBG_EEPROM, "%s: invalid mode 0x%x\n", __func__, mode);
			return false;
		}
		for (i = 0; i < nchan; i++) {
			pChannelData->channelValue = p_channels[i];

			EEREAD(off++);
			pChannelData->pcdacMax     = (u_int16_t)((eeval >> 10) & PCDAC_MASK);
			pChannelData->pcdacMin     = (u_int16_t)((eeval >> 4) & PCDAC_MASK);
			pChannelData->PwrValues[0] = (u_int16_t)((eeval << 2) & POWER_MASK);

			EEREAD(off++);
			pChannelData->PwrValues[0] |= (u_int16_t)((eeval >> 14) & 0x3);
			pChannelData->PwrValues[1] = (u_int16_t)((eeval >> 8) & POWER_MASK);
			pChannelData->PwrValues[2] = (u_int16_t)((eeval >> 2) & POWER_MASK);
			pChannelData->PwrValues[3] = (u_int16_t)((eeval << 4) & POWER_MASK);

			EEREAD(off++);
			pChannelData->PwrValues[3] |= (u_int16_t)((eeval >> 12) & 0xf);
			pChannelData->PwrValues[4] = (u_int16_t)((eeval >> 6) & POWER_MASK);
			pChannelData->PwrValues[5] = (u_int16_t)(eeval  & POWER_MASK);

			EEREAD(off++);
			pChannelData->PwrValues[6] = (u_int16_t)((eeval >> 10) & POWER_MASK);
			pChannelData->PwrValues[7] = (u_int16_t)((eeval >> 4) & POWER_MASK);
			pChannelData->PwrValues[8] = (u_int16_t)((eeval << 2) & POWER_MASK);

			EEREAD(off++);
			pChannelData->PwrValues[8] |= (u_int16_t)((eeval >> 14) & 0x3);
			pChannelData->PwrValues[9] = (u_int16_t)((eeval >> 8) & POWER_MASK);
			pChannelData->PwrValues[10] = (u_int16_t)((eeval >> 2) & POWER_MASK);

			getPcdacInterceptsFromPcdacMinMax(ee,
				pChannelData->pcdacMin, pChannelData->pcdacMax,
				pChannelData->PcdacValues) ;

			for (j = 0; j < pChannelData->numPcdacValues; j++) {
				pChannelData->PwrValues[j] = (u_int16_t)(
					PWR_STEP * pChannelData->PwrValues[j]);
				/* Note these values are scaled up. */
			}
			pChannelData++;
		}
	}
	return true;
#undef EEREAD
}

/*
 * Copy EEPROM Target Power Calbration per rate contents 
 * into the allocated space
 */
static bool
readEepromTargetPowerCalInfo(struct ath_hal *ah, HAL_EEPROM *ee)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	u_int16_t eeval;
	u_int32_t off;
        u_int16_t enable24;
	int i, mode, nchan;

        enable24 = ee->ee_Bmode || ee->ee_Gmode;
	for (mode = headerInfo11A; mode <= headerInfo11G; mode++) {
		TRGT_POWER_INFO *pPowerInfo;
		u_int16_t *pNumTrgtChannels;

		off = ee->ee_version >= AR_EEPROM_VER4_0 ?
				ee->ee_targetPowersStart - GROUP5_OFFSET :
		      ee->ee_version >= AR_EEPROM_VER3_3 ?
				GROUPS_OFFSET3_3 : GROUPS_OFFSET3_2;
		switch (mode) {
		case headerInfo11A:
			off += GROUP5_OFFSET;
			nchan = NUM_TEST_FREQUENCIES;
			pPowerInfo = ee->ee_trgtPwr_11a;
			pNumTrgtChannels = &ee->ee_numTargetPwr_11a;
			break;
		case headerInfo11B:
			if (!enable24)
				continue;
			off += GROUP6_OFFSET;
			nchan = 2;
			pPowerInfo = ee->ee_trgtPwr_11b;
			pNumTrgtChannels = &ee->ee_numTargetPwr_11b;
			break;
		case headerInfo11G:
			if (!enable24)
				continue;
			off += GROUP7_OFFSET;
			nchan = 3;
			pPowerInfo = ee->ee_trgtPwr_11g;
			pNumTrgtChannels = &ee->ee_numTargetPwr_11g;
			break;
		default:
			HDPRINTF(ah,HAL_DBG_EEPROM, "%s: invalid mode 0x%x\n", __func__, mode);
			return false;
		}
		*pNumTrgtChannels = 0;
		for (i = 0; i < nchan; i++) {
			EEREAD(off++);
			if (ee->ee_version >= AR_EEPROM_VER3_3) {
				pPowerInfo->testChannel = (eeval >> 8) & 0xff;
			} else {
				pPowerInfo->testChannel = (eeval >> 9) & 0x7f;
			}

			if (pPowerInfo->testChannel != 0) {
				/* get the channel value and read rest of info */
				if (mode == headerInfo11A) {
					pPowerInfo->testChannel = fbin2freq(ee, pPowerInfo->testChannel);
				} else {
					pPowerInfo->testChannel = fbin2freq_2p4(ee, pPowerInfo->testChannel);
				}

				if (ee->ee_version >= AR_EEPROM_VER3_3) {
					pPowerInfo->twicePwr6_24 = (eeval >> 2) & POWER_MASK;
					pPowerInfo->twicePwr36   = (eeval << 4) & POWER_MASK;
				} else {
					pPowerInfo->twicePwr6_24 = (eeval >> 3) & POWER_MASK;
					pPowerInfo->twicePwr36   = (eeval << 3) & POWER_MASK;
				}

				EEREAD(off++);
				if (ee->ee_version >= AR_EEPROM_VER3_3) {
					pPowerInfo->twicePwr36 |= (eeval >> 12) & 0xf;
					pPowerInfo->twicePwr48 = (eeval >> 6) & POWER_MASK;
					pPowerInfo->twicePwr54 =  eeval & POWER_MASK;
				} else {
					pPowerInfo->twicePwr36 |= (eeval >> 13) & 0x7;
					pPowerInfo->twicePwr48 = (eeval >> 7) & POWER_MASK;
					pPowerInfo->twicePwr54 = (eeval >> 1) & POWER_MASK;
				}
				(*pNumTrgtChannels)++;
			}
			pPowerInfo++;
		}
	}
	return true;
#undef EEREAD
}

/*
 * Now copy EEPROM Coformance Testing Limits contents 
 * into the allocated space
 */
static bool
readEepromCTLInfo(struct ath_hal *ah, HAL_EEPROM *ee)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	RD_EDGES_POWER *rep;
	u_int16_t eeval;
	u_int32_t off;
	int i, j;

	rep = ee->ee_rdEdgesPower;

	off = GROUP8_OFFSET +
		(ee->ee_version >= AR_EEPROM_VER4_0 ?
			ee->ee_targetPowersStart - GROUP5_OFFSET :
	         ee->ee_version >= AR_EEPROM_VER3_3 ?
			GROUPS_OFFSET3_3 : GROUPS_OFFSET3_2);
	for (i = 0; i < ee->ee_numCtls; i++) {
		if (ee->ee_ctl[i] == 0) {
			/* Move offset and edges */
			off += (ee->ee_version >= AR_EEPROM_VER3_3 ? 8 : 7);
			rep += NUM_EDGES;
			continue;
		}
		if (ee->ee_version >= AR_EEPROM_VER3_3) {
			for (j = 0; j < NUM_EDGES; j += 2) {
				EEREAD(off++);
				rep[j].rdEdge = (eeval >> 8) & FREQ_MASK_3_3;
				rep[j+1].rdEdge = eeval & FREQ_MASK_3_3;
			}
			for (j = 0; j < NUM_EDGES; j += 2) {
				EEREAD(off++);
				rep[j].twice_rdEdgePower = 
					(eeval >> 8) & POWER_MASK;
				rep[j].flag = (eeval >> 14) & 1;
				rep[j+1].twice_rdEdgePower = eeval & POWER_MASK;
				rep[j+1].flag = (eeval >> 6) & 1;
			}
		} else { 
			EEREAD(off++);
			rep[0].rdEdge = (eeval >> 9) & FREQ_MASK;
			rep[1].rdEdge = (eeval >> 2) & FREQ_MASK;
			rep[2].rdEdge = (eeval << 5) & FREQ_MASK;

			EEREAD(off++);
			rep[2].rdEdge |= (eeval >> 11) & 0x1f;
			rep[3].rdEdge = (eeval >> 4) & FREQ_MASK;
			rep[4].rdEdge = (eeval << 3) & FREQ_MASK;

			EEREAD(off++);
			rep[4].rdEdge |= (eeval >> 13) & 0x7;
			rep[5].rdEdge = (eeval >> 6) & FREQ_MASK;
			rep[6].rdEdge = (eeval << 1) & FREQ_MASK;

			EEREAD(off++);
			rep[6].rdEdge |= (eeval >> 15) & 0x1;
			rep[7].rdEdge = (eeval >> 8) & FREQ_MASK;

			rep[0].twice_rdEdgePower = (eeval >> 2) & POWER_MASK;
			rep[1].twice_rdEdgePower = (eeval << 4) & POWER_MASK;

			EEREAD(off++);
			rep[1].twice_rdEdgePower |= (eeval >> 12) & 0xf;
			rep[2].twice_rdEdgePower = (eeval >> 6) & POWER_MASK;
			rep[3].twice_rdEdgePower = eeval & POWER_MASK;

			EEREAD(off++);
			rep[4].twice_rdEdgePower = (eeval >> 10) & POWER_MASK;
			rep[5].twice_rdEdgePower = (eeval >> 4) & POWER_MASK;
			rep[6].twice_rdEdgePower = (eeval << 2) & POWER_MASK;

			EEREAD(off++);
			rep[6].twice_rdEdgePower |= (eeval >> 14) & 0x3;
			rep[7].twice_rdEdgePower = (eeval >> 8) & POWER_MASK;
		}

		for (j = 0; j < NUM_EDGES; j++ ) {
			if (rep[j].rdEdge != 0 || rep[j].twice_rdEdgePower != 0) {
				if ((ee->ee_ctl[i] & CTL_MODE_M) == CTL_11A ||
				    (ee->ee_ctl[i] & CTL_MODE_M) == CTL_TURBO) {
					rep[j].rdEdge = fbin2freq(ee, rep[j].rdEdge);
				} else {
					rep[j].rdEdge = fbin2freq_2p4(ee, rep[j].rdEdge);
				}
			}
		}
		rep += NUM_EDGES;
	}
	return true;
#undef EEREAD
}

/*
 * Read the individual header fields for a Rev 3 EEPROM
 */
static bool
readHeaderInfo(struct ath_hal *ah, HAL_EEPROM *ee)
{
#define	EEREAD(_off) do {				\
	if (!ath_hal_eepromRead(ah, _off, &eeval))	\
		return false;			\
} while (0)
	static const u_int32_t headerOffset3_0[] = {
		0x00C2, /* 0 - Mode bits, device type, max turbo power */
		0x00C4, /* 1 - 2.4 and 5 antenna gain */
		0x00C5, /* 2 - Begin 11A modal section */
		0x00D0, /* 3 - Begin 11B modal section */
		0x00DA, /* 4 - Begin 11G modal section */
		0x00E4  /* 5 - Begin CTL section */
	};
	static const u_int32_t headerOffset3_3[] = {
		0x00C2, /* 0 - Mode bits, device type, max turbo power */
		0x00C3, /* 1 - 2.4 and 5 antenna gain */
		0x00D4, /* 2 - Begin 11A modal section */
		0x00F2, /* 3 - Begin 11B modal section */
		0x010D, /* 4 - Begin 11G modal section */
		0x0128  /* 5 - Begin CTL section */
	};

	static const u_int32_t regCapOffsetPre4_0 = 0x00CF;
	static const u_int32_t regCapOffsetPost4_0 = 0x00CA; 

	const u_int32_t *header;
	u_int32_t off;
	u_int16_t eeval;
	int i;

	/* initialize cckOfdmGainDelta for < 4.2 eeprom */
	ee->ee_cckOfdmGainDelta = CCK_OFDM_GAIN_DELTA;
	ee->ee_scaledCh14FilterCckDelta = TENX_CH14_FILTER_CCK_DELTA_INIT;

	if (ee->ee_version >= AR_EEPROM_VER3_3) {
		header = headerOffset3_3;
		ee->ee_numCtls = NUM_CTLS_3_3;
	} else {
		header = headerOffset3_0;
		ee->ee_numCtls = NUM_CTLS;
	}

	EEREAD(header[0]);
	ee->ee_turbo5Disable	= (eeval >> 15) & 0x01;
	ee->ee_rfKill		= (eeval >> 14) & 0x01;
	ee->ee_deviceType	= AH_PRIVATE(ah)->ah_devType = (u_int16_t)(eeval >> 11) & 0x07;
	ee->ee_turbo2WMaxPower5	= (eeval >> 4) & 0x7F;
	if (ee->ee_version >= AR_EEPROM_VER4_0)
		ee->ee_turbo2Disable	= (eeval >> 3) & 0x01;
	else
#ifndef ATH_SUPERG_DYNTURBO        
		ee->ee_turbo2Disable	= 1;
#else
		ee->ee_turbo2Disable	= (AH_PRIVATE(ah)->ah_config.ath_hal_disableTurboG == USE_ABOLT) ?
                                   0 : 1;
#endif                                
	ee->ee_Gmode		= (eeval >> 2) & 0x01;
	ee->ee_Bmode		= (eeval >> 1) & 0x01;
	ee->ee_Amode		= (eeval & 0x01);

	off = header[1];
	EEREAD(off++);
	ee->ee_antennaGainMax[0] = (int8_t)((eeval >> 8) & 0xFF);
	ee->ee_antennaGainMax[1] = (int8_t)(eeval & 0xFF);
	if (ee->ee_version >= AR_EEPROM_VER4_0) {
		EEREAD(off++);
		ee->ee_eepMap		 = (eeval>>14) & 0x3;
		ee->ee_disableXr5	 = (eeval>>13) & 0x1;
		ee->ee_disableXr2	 = (eeval>>12) & 0x1;
		ee->ee_earStart		 = eeval & 0xfff;

		EEREAD(off++);
		ee->ee_targetPowersStart = eeval & 0xfff;
		ee->ee_exist32kHzCrystal = (eeval>>14) & 0x1;

		if (ee->ee_version >= AR_EEPROM_VER5_0) {
			off += 2;
			EEREAD(off);
			ee->ee_eepMap2PowerCalStart = (eeval >> 4) & 0xfff;
			/* Properly cal'ed 5.0 devices should be non-zero */
		}
	}

	/* Read the moded sections of the EEPROM header in the order A, B, G */
	for (i = headerInfo11A; i <= headerInfo11G; i++) {
		/* Set the offset via the index */
		off = header[2 + i];

		EEREAD(off++);
		ee->ee_switchSettling[i] = (eeval >> 8) & 0x7f;
		ee->ee_txrxAtten[i] = (eeval >> 2) & 0x3f;
		ee->ee_antennaControl[0][i] = (eeval << 4) & 0x3f;

		EEREAD(off++);
		ee->ee_antennaControl[0][i] |= (eeval >> 12) & 0x0f;
		ee->ee_antennaControl[1][i] = (eeval >> 6) & 0x3f;
		ee->ee_antennaControl[2][i] = eeval & 0x3f;

		EEREAD(off++);
		ee->ee_antennaControl[3][i] = (eeval >> 10)  & 0x3f;
		ee->ee_antennaControl[4][i] = (eeval >> 4)  & 0x3f;
		ee->ee_antennaControl[5][i] = (eeval << 2)  & 0x3f;

		EEREAD(off++);
		ee->ee_antennaControl[5][i] |= (eeval >> 14)  & 0x03;
		ee->ee_antennaControl[6][i] = (eeval >> 8)  & 0x3f;
		ee->ee_antennaControl[7][i] = (eeval >> 2)  & 0x3f;
		ee->ee_antennaControl[8][i] = (eeval << 4)  & 0x3f;

		EEREAD(off++);
		ee->ee_antennaControl[8][i] |= (eeval >> 12)  & 0x0f;
		ee->ee_antennaControl[9][i] = (eeval >> 6)  & 0x3f;
		ee->ee_antennaControl[10][i] = eeval & 0x3f;

		EEREAD(off++);
		ee->ee_adcDesiredSize[i] = (int8_t)((eeval >> 8)  & 0xff);
		switch (i) {
		case headerInfo11A:
			ee->ee_ob4 = (eeval >> 5)  & 0x07;
			ee->ee_db4 = (eeval >> 2)  & 0x07;
			ee->ee_ob3 = (eeval << 1)  & 0x07;
			break;
		case headerInfo11B:
			ee->ee_obFor24 = (eeval >> 4)  & 0x07;
			ee->ee_dbFor24 = eeval & 0x07;
			break;
		case headerInfo11G:
			ee->ee_obFor24g = (eeval >> 4)  & 0x07;
			ee->ee_dbFor24g = eeval & 0x07;
			break;
		}

		if (i == headerInfo11A) {
			EEREAD(off++);
			ee->ee_ob3 |= (eeval >> 15)  & 0x01;
			ee->ee_db3 = (eeval >> 12)  & 0x07;
			ee->ee_ob2 = (eeval >> 9)  & 0x07;
			ee->ee_db2 = (eeval >> 6)  & 0x07;
			ee->ee_ob1 = (eeval >> 3)  & 0x07;
			ee->ee_db1 = eeval & 0x07;
		}

		EEREAD(off++);
		ee->ee_txEndToXLNAOn[i] = (eeval >> 8)  & 0xff;
		ee->ee_thresh62[i] = eeval & 0xff;

		EEREAD(off++);
		ee->ee_txEndToXPAOff[i] = (eeval >> 8)  & 0xff;
		ee->ee_txFrameToXPAOn[i] = eeval  & 0xff;

		EEREAD(off++);
		ee->ee_pgaDesiredSize[i] = (int8_t)((eeval >> 8)  & 0xff);
		ee->ee_noiseFloorThresh[i] = eeval  & 0xff;
		if (ee->ee_noiseFloorThresh[i] & 0x80) {
			ee->ee_noiseFloorThresh[i] = 0 -
				((ee->ee_noiseFloorThresh[i] ^ 0xff) + 1);
		}

		EEREAD(off++);
		ee->ee_xlnaGain[i] = (eeval >> 5)  & 0xff;
		ee->ee_xgain[i] = (eeval >> 1)  & 0x0f;
		ee->ee_xpd[i] = eeval  & 0x01;
		if (ee->ee_version >= AR_EEPROM_VER4_0) {
			switch (i) {
			case headerInfo11A:
				ee->ee_fixedBias5 = (eeval >> 13) & 0x1;
				break;
			case headerInfo11G:
				ee->ee_fixedBias2 = (eeval >> 13) & 0x1;
				break;
			}
		}

		if (ee->ee_version >= AR_EEPROM_VER3_3) {
			EEREAD(off++);
			ee->ee_falseDetectBackoff[i] = (eeval >> 6) & 0x7F;
			switch (i) {
			case headerInfo11B:
				ee->ee_ob2GHz[0] = eeval & 0x7;
				ee->ee_db2GHz[0] = (eeval >> 3) & 0x7;
				break;
			case headerInfo11G:
				ee->ee_ob2GHz[1] = eeval & 0x7;
				ee->ee_db2GHz[1] = (eeval >> 3) & 0x7;
				break;
			case headerInfo11A:
				ee->ee_xrTargetPower5 = eeval & 0x3f;
				break;
			}
		}
		if (ee->ee_version >= AR_EEPROM_VER3_4) {
			ee->ee_gainI[i] = (eeval >> 13) & 0x07;

			EEREAD(off++);
			ee->ee_gainI[i] |= (eeval << 3) & 0x38;
			if (i == headerInfo11G) {
				ee->ee_cckOfdmPwrDelta = (eeval >> 3) & 0xFF;
				if (ee->ee_version >= AR_EEPROM_VER4_6)
					ee->ee_scaledCh14FilterCckDelta =
						(eeval >> 11) & 0x1f;
			}
			if (i == headerInfo11A &&
			    ee->ee_version >= AR_EEPROM_VER4_0) {
				ee->ee_iqCalI[0] = (eeval >> 8 ) & 0x3f;
				ee->ee_iqCalQ[0] = (eeval >> 3 ) & 0x1f;
			}
		} else {
			ee->ee_gainI[i] = 10;
			ee->ee_cckOfdmPwrDelta = TENX_OFDM_CCK_DELTA_INIT;
		}
		if (ee->ee_version >= AR_EEPROM_VER4_0) {
			switch (i) {
			case headerInfo11B:
				EEREAD(off++);
				ee->ee_calPier11b[0] =
					fbin2freq_2p4(ee, eeval&0xff);
				ee->ee_calPier11b[1] =
					fbin2freq_2p4(ee, (eeval >> 8)&0xff);
				EEREAD(off++);
				ee->ee_calPier11b[2] =
					fbin2freq_2p4(ee, eeval&0xff);
				if (ee->ee_version >= AR_EEPROM_VER4_1)
					ee->ee_rxtxMargin[headerInfo11B] =
						(eeval >> 8) & 0x3f;
				break;
			case headerInfo11G:
				EEREAD(off++);
				ee->ee_calPier11g[0] =
					fbin2freq_2p4(ee, eeval & 0xff);
				ee->ee_calPier11g[1] =
					fbin2freq_2p4(ee, (eeval >> 8) & 0xff);

				EEREAD(off++);
				ee->ee_turbo2WMaxPower2 = eeval & 0x7F;
				ee->ee_xrTargetPower2 = (eeval >> 7) & 0x3f;

				EEREAD(off++);
				ee->ee_calPier11g[2] =
					fbin2freq_2p4(ee, eeval & 0xff);
				if (ee->ee_version >= AR_EEPROM_VER4_1)
					 ee->ee_rxtxMargin[headerInfo11G] =
						(eeval >> 8) & 0x3f;

				EEREAD(off++);
				ee->ee_iqCalI[1] = (eeval >> 5) & 0x3F;
				ee->ee_iqCalQ[1] = eeval & 0x1F;

				if (ee->ee_version >= AR_EEPROM_VER4_2) {
					EEREAD(off++);
					ee->ee_cckOfdmGainDelta =
						(u_int8_t)(eeval & 0xFF);
					if (ee->ee_version >= AR_EEPROM_VER5_0) {
						ee->ee_switchSettlingTurbo[1] =
							(eeval >> 8) & 0x7f;
						ee->ee_txrxAttenTurbo[1] =
							(eeval >> 15) & 0x1;
						EEREAD(off++);
						ee->ee_txrxAttenTurbo[1] |=
							(eeval & 0x1F) << 1;
						ee->ee_rxtxMarginTurbo[1] =
							(eeval >> 5) & 0x3F;
						ee->ee_adcDesiredSizeTurbo[1] =
							(eeval >> 11) & 0x1F;
						EEREAD(off++);
						ee->ee_adcDesiredSizeTurbo[1] |=
							(eeval & 0x7) << 5;
						ee->ee_pgaDesiredSizeTurbo[1] =
							(eeval >> 3) & 0xFF;
					}
				}
				break;
			case headerInfo11A:
				if (ee->ee_version >= AR_EEPROM_VER4_1) {
					EEREAD(off++);
					ee->ee_rxtxMargin[headerInfo11A] =
						eeval & 0x3f;
					if (ee->ee_version >= AR_EEPROM_VER5_0) {
						ee->ee_switchSettlingTurbo[0] =
							(eeval >> 6) & 0x7f;
						ee->ee_txrxAttenTurbo[0] =
							(eeval >> 13) & 0x7;
						EEREAD(off++);
						ee->ee_txrxAttenTurbo[0] |=
							(eeval & 0x7) << 3;
						ee->ee_rxtxMarginTurbo[0] =
							(eeval >> 3) & 0x3F;
						ee->ee_adcDesiredSizeTurbo[0] =
							(eeval >> 9) & 0x7F;
						EEREAD(off++);
						ee->ee_adcDesiredSizeTurbo[0] |=
							(eeval & 0x1) << 7;
						ee->ee_pgaDesiredSizeTurbo[0] =
							(eeval >> 1) & 0xFF;
					}
				}
				break;
			}
		}
	}
	if (ee->ee_version < AR_EEPROM_VER3_3) {
		/* Version 3.1+ specific parameters */
		EEREAD(0xec);
		ee->ee_ob2GHz[0] = eeval & 0x7;
		ee->ee_db2GHz[0] = (eeval >> 3) & 0x7;

		EEREAD(0xed);
		ee->ee_ob2GHz[1] = eeval & 0x7;
		ee->ee_db2GHz[1] = (eeval >> 3) & 0x7;
	}

	/* Initialize corner cal (thermal tx gain adjust parameters) */
	ee->ee_cornerCal.clip = 4;
	ee->ee_cornerCal.pd90 = 1;
	ee->ee_cornerCal.pd84 = 1;
	ee->ee_cornerCal.gSel = 0;

	/*
	* Read the conformance test limit identifiers
	* These are used to match regulatory domain testing needs with
	* the RD-specific tests that have been calibrated in the EEPROM.
	*/
	off = header[5];
	for (i = 0; i < ee->ee_numCtls; i += 2) {
		EEREAD(off++);
		ee->ee_ctl[i] = (eeval >> 8) & 0xff;
		ee->ee_ctl[i+1] = eeval & 0xff;
	}

	/* Read spur mitigation data */
	if (ee->ee_version >= AR_EEPROM_VER5_3) {
		for (i = 0; i < AR_EEPROM_MODAL_SPURS; i++) {
			EEREAD(off);
			ee->ee_spurChans[i][0] = eeval;
			EEREAD(off+AR_EEPROM_MODAL_SPURS);
			ee->ee_spurChans[i][1] = eeval;
			off++;
		}
	}

	/* WAR for recent changes to NF scale */
	if (ee->ee_version <= AR_EEPROM_VER3_2) {
		ee->ee_noiseFloorThresh[headerInfo11A] = -54;
		ee->ee_noiseFloorThresh[headerInfo11B] = -1;
		ee->ee_noiseFloorThresh[headerInfo11G] = -1;
	}
	/* WAR to override thresh62 for better 2.4 and 5 operation */
	if (ee->ee_version <= AR_EEPROM_VER3_2) {
		ee->ee_thresh62[headerInfo11A] = 15;	/* 11A */
		ee->ee_thresh62[headerInfo11B] = 28;	/* 11B */
		ee->ee_thresh62[headerInfo11G] = 28;	/* 11G */
	}

	/* Check for regulatory capabilities */
	if (ee->ee_version >= AR_EEPROM_VER4_0) {
		EEREAD(regCapOffsetPost4_0);
	} else {
		EEREAD(regCapOffsetPre4_0);
	}

	ee->ee_regCap = eeval;

	if (ee->ee_Amode == 0) {
		/* Check for valid Amode in upgraded h/w */
		if (ee->ee_version >= AR_EEPROM_VER4_0) {
			ee->ee_Amode = (ee->ee_regCap & AR_EEPROM_EEREGCAP_EN_KK_NEW_11A)?1:0;
		} else {
			ee->ee_Amode = (ee->ee_regCap & AR_EEPROM_EEREGCAP_EN_KK_NEW_11A_PRE4_0)?1:0;
		}
	}

	return true;
#undef EEREAD
}

/*
 * Now verify and copy EEPROM contents into the allocated space
 */
bool
ath_hal_readEepromIntoDataset(struct ath_hal *ah, HAL_EEPROM *ee)
{
	/* Read the header information here */
	if (!readHeaderInfo(ah, ee))
		return false;
#if 0
	/* Require 5112 devices to have EEPROM 4.0 EEP_MAP set */
	if (IS_5112(ah) && !ee->ee_eepMap) {
		HDPRINTF(ah, HAL_DBG_EEPROM "%s: 5112 devices must have EEPROM 4.0 with the "
			"EEP_MAP set\n", __func__);
		return false;
	}
#endif
	/*
	 * Group 1: frequency pier locations readback
	 * check that the structure has been populated
	 * with enough space to hold the channels
	 *
	 * NOTE: Group 1 contains the 5 GHz channel numbers
	 *	 that have dBm->pcdac calibrated information.
	 */
	if (!readEepromFreqPierInfo(ah, ee))
		return false;

	/*
	 * Group 2:  readback data for all frequency piers
	 *
	 * NOTE: Group 2 contains the raw power calibration
	 *	 information for each of the channels that we
	 *	 recorded above.
	 */
	if (!readEepromRawPowerCalInfo(ah, ee))
		return false;

	/*
	 * Group 5: target power values per rate
	 *
	 * NOTE: Group 5 contains the recorded maximum power
	 *	 in dB that can be attained for the given rate.
	 */
	/* Read the power per rate info for test channels */
	if (!readEepromTargetPowerCalInfo(ah, ee))
		return false;

	/*
	 * Group 8: Conformance Test Limits information
	 *
	 * NOTE: Group 8 contains the values to limit the
	 *	 maximum transmit power value based on any
	 *	 band edge violations.
	 */
	/* Read the RD edge power limits */
	return readEepromCTLInfo(ah, ee);
}


/*
 * Reclaim any storage allocated by ath_hal_readEepromIntoDataset.
 */
void
ath_hal_eepromDetach(struct ath_hal *ah, HAL_EEPROM *ee)
{
        if (ee->ee_version >= AR_EEPROM_VER4_0 && ee->ee_eepMap == 1)
		freeEepromRawPowerCalInfo5112(ah, ee);
}
