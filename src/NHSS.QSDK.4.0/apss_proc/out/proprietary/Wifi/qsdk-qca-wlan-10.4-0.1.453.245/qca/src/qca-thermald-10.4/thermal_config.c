/*===========================================================================

  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "thermal.h"

#define CONFIG_FILE_SIZE_MAX  5120


static char *field_names[] = {
	"debug",
	"sampling",
	"thresholds",
	"thresholds_clr",
	"actions",
	"action_info",
};

enum {
	DEBUG,
	SAMPLING,
	THRESHOLDS,
	THRESHOLDS_CLR,
	ACTIONS,
	ACTION_INFO,
	FIELD_IDX_MAX
} field_type;

static char *action_names[] = {
	"none",
	"cpu",
	"shutdown",
#ifdef IPQ_806x
        "powersave",
	"nss",
#else
	"report",
	"lcd",
	"modem",
	"fusion",
	"battery",
	"gpu",
	"wlan",
	"camera",
	"camcorder"
#endif
};

sensor_setting_t def_ipq8064 = {
	.desc = "tsens_tz_sensor0",
	.id = 0,
	.disabled = 0, /* Sensor enabled */
	.sampling_period_us = 1000,
	.num_thresholds = 5, /* No. of threshold levels */

	.t = {
		{
			.lvl_trig = 105, /* Level trigger temp */
			.lvl_clr = 95, /* Level clear temp */
			.num_actions = 1, /* No. of actions */
			.actions = {
				{
					.action = CPU_FREQ, /* Action type */
					.info = 1200000 /* Action info/value */
				}
			}
		},

		{
			.lvl_trig = 110,
			.lvl_clr = 100,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 1000000
				}
			}
		},

		{
			.lvl_trig = 115,
			.lvl_clr = 105,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 800000
				}
			}
		},

		{
			.lvl_trig = 120,
			.lvl_clr = 110,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 384000
				}
			}
		},

		{
			.lvl_trig = 125,
			.lvl_clr = 115,
			.num_actions = 1,
			.actions = {
				{
					.action = SHUTDOWN,
					.info = 1000
				}
			}
		}
	},

	/*
	Internal variables initialized with threshold count
	*/
	._n_thresholds = 5,
	._n_to_clear = 5,
	._n_actions = 5,
	._n_action_info = 5

};

sensor_setting_t def_ipq8066 = {
	.desc = "tsens_tz_sensor0",
	.id = 0,
	.disabled = 0,
	.sampling_period_us = 1000,
	.num_thresholds = 3,

	.t = {
		{
			.lvl_trig = 115,
			.lvl_clr = 105,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 800000
				}
			}
		},

		{
			.lvl_trig = 120,
			.lvl_clr = 110,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 384000
				}
			}
		},

		{
			.lvl_trig = 125,
			.lvl_clr = 115,
			.num_actions = 1,
			.actions = {
				{
					.action = SHUTDOWN,
					.info = 1000
				}
			}
		}
	},

	._n_thresholds = 3,
	._n_to_clear = 3,
	._n_actions = 3,
	._n_action_info = 3

};

sensor_setting_t def_ipq8069 = {
	.desc = "tsens_tz_sensor0",
	.id = 0,
	.disabled = 0,
	.sampling_period_us = 1000,
	.num_thresholds = 3,

	.t = {
		{
			.lvl_trig = 115,
			.lvl_clr = 105,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 1400000
				}
			}
		},

		{
			.lvl_trig = 120,
			.lvl_clr = 110,
			.num_actions = 1,
			.actions = {
				{
					.action = CPU_FREQ,
					.info = 800000
				}
			}
		},

		{
			.lvl_trig = 125,
			.lvl_clr = 115,
			.num_actions = 1,
			.actions = {
				{
					.action = SHUTDOWN,
					.info = 1000
				}
			}
		}
	},

	._n_thresholds = 3,
	._n_to_clear = 3,
	._n_actions = 3,
	._n_action_info = 3
};

void init_settings(thermal_setting_t *settings)
{
	int i;

	if (!settings)
		return;

	settings->sample_period_ms = -1;
	for (i = 0; i < SENSOR_IDX_MAX; i++) {
		settings->sensors[i].sampling_period_us = -1;
		settings->sensors[i].num_thresholds     = 0;
		settings->sensors[i].desc               = NULL;
		settings->sensors[i].id                 = i;
		settings->sensors[i].disabled           = 1;
		settings->sensors[i].action_mask        = 0;
		settings->sensors[i]._n_thresholds      = 0;
		settings->sensors[i]._n_to_clear        = 0;
		settings->sensors[i]._n_actions         = 0;
		settings->sensors[i]._n_action_info     = 0;
		settings->sensors[i].last_lvl = 0;
	}
}


void update_def_sensor_settings(thermal_setting_t *settings)
{
	sensor_setting_t *def_sensor_setting;
	int soc_id;
	int i,j;

	if (!settings)
                return;

	msg("Using default sensor setting...\n");

	settings->sample_period_ms = 5000;
	soc_id = therm_get_msm_id();

	dbgmsg("Identidied SOC ID: %d \n", soc_id);

	switch (soc_id) {
		case THERM_IPQ_8062:
		case THERM_IPQ_8066:
			def_sensor_setting = &def_ipq8066;
			break;

		case THERM_IPQ_8064:
		case THERM_IPQ_8068:
			def_sensor_setting = &def_ipq8064;
			break;

		case THERM_IPQ_8065:
		case THERM_IPQ_8069:
			def_sensor_setting = &def_ipq8069;
			break;

		default:
			def_sensor_setting = &def_ipq8064;
	}

	settings->sensors[0].desc = def_sensor_setting->desc;
	settings->sensors[0].id = def_sensor_setting->id;
	settings->sensors[0].disabled = def_sensor_setting->disabled;
	settings->sensors[0].sampling_period_us = def_sensor_setting->sampling_period_us;
	settings->sensors[0].num_thresholds = def_sensor_setting->num_thresholds;
	settings->sensors[0]._n_thresholds = def_sensor_setting->_n_thresholds;
	settings->sensors[0]._n_to_clear = def_sensor_setting->_n_to_clear;
	settings->sensors[0]._n_actions = def_sensor_setting->_n_actions;
	settings->sensors[0]._n_action_info = def_sensor_setting->_n_action_info;

	for (i = 0; i < def_sensor_setting->num_thresholds; i++)
	{
		settings->sensors[0].t[i].lvl_trig = CONV(def_sensor_setting->t[i].lvl_trig);
		settings->sensors[0].t[i].lvl_clr = CONV(def_sensor_setting->t[i].lvl_clr);
		settings->sensors[0].t[i].num_actions = def_sensor_setting->t[i].num_actions;

		for (j = 0; j < def_sensor_setting->t[i].num_actions; j++)
		{
			settings->sensors[0].t[i].actions[j].action = def_sensor_setting->t[i].actions[j].action;
			settings->sensors[0].t[i].actions[j].info = def_sensor_setting->t[i].actions[j].info;
		}
	}
}

void skip_space(char **ppos)
{
	char *pos = *ppos;

	while(*pos != '\0') {
		if ((*pos == ' ') || (*pos == '\t') || (*pos == '\n')) {
			pos++;
		}
		else
			break;
	}
	*ppos = pos;
}

void skip_line(char **ppos)
{
	char *pos = *ppos;
	char *ptmp;

	ptmp = index(pos, '\n');
	if (!ptmp) {
		*pos = '\0';
		return;
	}

	*ppos = ptmp + 1;
}

int parse_action(char *pvalue)
{
	int i;

	for (i = 0; i < ACTION_IDX_MAX; i++) {
		if (strcasecmp(action_names[i], pvalue) == 0)
			return i;
	}

	/* Unknown action; assuming none */
	msg("Unknown action '%s'\n", pvalue);
	return NONE;
}

int parse_action_values(char **ppos, double result[THRESHOLDS_MAX][ACTIONS_MAX], int *num_actions, int is_action)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	int  nresult = 0;

	ptmp = index(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		char *pstr = NULL;
		char *psave2 = NULL;
		int count = 0;

		pstr = strtok_r(pvalue , "+", &psave2);
		while (pstr != NULL) {
			dbgmsg("Found action '%s'\n", pstr);

			if (!is_action)
				result[nresult][count] = strtod(pstr, NULL);
			else
				result[nresult][count] = parse_action(pstr);
			count++;
			if (count >= ACTIONS_MAX)
				break;

			pstr = strtok_r(NULL, "+", &psave2);
		}

		num_actions[nresult++] = count;
		if (nresult >= THRESHOLDS_MAX)
			break;
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}

	dbgmsg("No. of items found : %d\n", nresult);

	*ppos = ptmp + 1;
	return nresult;
}

int parse_values(char **ppos, double *result)
{
	char *pos = *ppos;
	char *ptmp;
	char *pvalue, *psave = NULL;
	int  nresult = 0;

	ptmp = index(pos, '\n');
	if (ptmp)
		*ptmp = '\0';

	pvalue = strtok_r(pos, "\t \r\n", &psave);

	while (pvalue != NULL) {
		result[nresult] = strtod(pvalue, NULL);
		nresult++;
		if (nresult >= THRESHOLDS_MAX)
			break;
		pvalue = strtok_r(NULL, "\t \r\n", &psave);
	}

	*ppos = ptmp + 1;
	return nresult;
}

int parse_config(thermal_setting_t *settings, int fd)
{
	char buf[CONFIG_FILE_SIZE_MAX];
	int sz, i;
	char *pos = buf;
	char *maxpos;
	int error_found = 0;
	char *idx;
	sensor_setting_t *in_section = NULL;
	int in_field = FIELD_IDX_MAX;

	if (fd == -1)
		return 0;

	dbgmsg("Loading configuration file...\n");

	sz = read(fd, buf, CONFIG_FILE_SIZE_MAX);
	if (sz < 0) {
		msg("Failed to read config file\n");
		return 0;
	}
	maxpos = pos + sz;
	dbgmsg("Config file read (%d bytes)\n", sz);

	buf[CONFIG_FILE_SIZE_MAX-1] = '\0';

	dbgmsg("Max sensor ID = %d\n" , SENSOR_IDX_MAX );
	for (i = 0; i < SENSOR_IDX_MAX; i++) {
		dbgmsg("Supported Sensors: %s\n", sensor_names[i] );
        }


	while (pos && (*pos != '\0') && (pos < maxpos)) {
		switch (*pos) {
		case '[':
			idx = index(++pos, ']');
			if (!idx) {
				error_found = 1;
				break;
			}
			in_field = FIELD_IDX_MAX;
			*idx = '\0';
			for (i = 0; i < SENSOR_IDX_MAX; i++) {
				if (strncasecmp(pos, sensor_names[i],
						SENSOR_NAME_MAX) != 0)
					continue;
				in_section = &settings->sensors[i];
				in_section->desc = sensor_names[i];
				pos = idx + 1;
				dbgmsg("Found section '%s'\n", in_section->desc);
				break;
			}
			if (i == SENSOR_IDX_MAX) {
				msg("Ignoring unknown section '%s'\n", pos);
				in_section = NULL;
				pos = index(++idx, '[');
				if (!pos)
					pos = &buf[CONFIG_FILE_SIZE_MAX-1];
			}
			break;

		case '\t':
		case '\r':
		case '\n':
		case ' ':
			pos++;
			break;

		default:
			if (in_field != FIELD_IDX_MAX) {
				int    j;
				int    num;
				double values[THRESHOLDS_MAX];
				int    num_actions[THRESHOLDS_MAX];
				double actions[THRESHOLDS_MAX][ACTIONS_MAX];

				switch (in_field) {
				case DEBUG:
					info("Debug output enabled from config");
					debug_output = 1;
					break;
				case SAMPLING:
					num = parse_values(&pos, values);
					if (num < 1)
						break;
					if (values[0] < SAMPLING_MS_MINIMUM) {
						values[0] = SAMPLING_MS_MINIMUM;
						info("Sampling time specified too low, using %d ms",
						    SAMPLING_MS_MINIMUM);
					}
					if (!in_section)
						settings->sample_period_ms = (int)values[0];
					else
						in_section->sampling_period_us = (int)values[0]*1000;
					break;
				case THRESHOLDS:
					if (!in_section)
						break;
					num = parse_values(&pos, values);
					if (num < 1)
						break;
					in_section->_n_thresholds = num;
					for (i = 0; i < num; i++) {
						if (strncmp(in_section->desc, "bcl", strlen("bcl")) != 0)
							in_section->t[i].lvl_trig = CONV(values[i]);
						else
							in_section->t[i].lvl_trig = values[i];
					}
					break;
				case THRESHOLDS_CLR:
					if (!in_section)
						break;
					num = parse_values(&pos, values);
					if (num < 1)
						break;
					in_section->_n_to_clear = num;
					for (i = 0; i < num; i++) {
						if (strncmp(in_section->desc, "bcl", strlen("bcl")) != 0)
							in_section->t[i].lvl_clr = CONV(values[i]);
						else
							in_section->t[i].lvl_clr = values[i];
					}
					break;
				case ACTIONS:
					if (!in_section)
						break;
					num = parse_action_values(&pos, actions, num_actions, 1);
					if (num < 1)
						break;
					in_section->_n_actions = num;
					for (i = 0; i < num; i++) {
						in_section->t[i].num_actions = num_actions[i];
						for (j = 0; j < num_actions[i]; j++) {
							in_section->t[i].actions[j].action = (int)actions[i][j];
							info("Threshold %d Action[%d] : %s\n", i, j, action_names[(int)actions[i][j]]);
						}
					}
					break;
				case ACTION_INFO:
					if (!in_section)
						break;
					num = parse_action_values(&pos, actions, num_actions, 0);
					if (num < 1)
						break;
					in_section->_n_action_info  = num;
					for (i = 0; i < num; i++) {
						for (j = 0; j < num_actions[i]; j++) {
							in_section->t[i].actions[j].info = (int)actions[i][j];
							info("Threshold %d Action Info[%d] : %lf\n", i, j, actions[i][j]);
						}
					}
					break;
				}
				in_field = FIELD_IDX_MAX;
				break;
			}

			idx = strpbrk(pos, "\t\r\n ");
			if (!idx) {
				msg("Error in file\n");
				error_found = 1;
				break;
			}
			*idx = '\0';
			for (i = 0; i < FIELD_IDX_MAX; i++) {
				if (strcasecmp(pos, field_names[i]) != 0)
					continue;
				pos = idx + 1;
				skip_space(&pos);
				dbgmsg("Found field '%s'\n", field_names[i]);
				in_field = i;
				break;
			}
			if (i == FIELD_IDX_MAX) {
				msg("Ignoring unknown field '%s'\n", pos);
				pos = idx + 1;
				skip_line(&pos);
			}
			break;
		}

		if (error_found) {
			return 0;
		}
	}
	return 1;
}

int threshold_array_compare(const void *x, const void *y)
{
	threshold_t *i = (threshold_t *)x;
	threshold_t *j = (threshold_t *)y;

	if (i->lvl_trig < j->lvl_trig)
		return -1;
	else if (i->lvl_trig > j->lvl_trig)
		return 1;
	return 0;
}

int bcl_threshold_array_compare(const void *x, const void *y)
{
	threshold_t *i = (threshold_t *)x;
	threshold_t *j = (threshold_t *)y;

	if (i->lvl_trig < j->lvl_trig)
		return 1;
	else if (i->lvl_trig > j->lvl_trig)
		return -1;
	return 0;
}

static void fallback_sampling_values(thermal_setting_t *settings,
				     sensor_setting_t * section)
{
	/* Fallback to default sampling times if sampling period
	   was not configured */
	if (settings == NULL ||
	    section == NULL) {
		msg("%s: unexpected NULL", __func__);
		return;
	}

	if (settings->sample_period_ms > 0 &&
	    section->sampling_period_us == -1) {
		section->sampling_period_us =
			settings->sample_period_ms * 1000;
		dbgmsg("Using configured default sampling period "
		       "%dms for sensor[%s]\n", settings->sample_period_ms,
		       section->desc);
	} else if (settings->sample_period_ms == -1 &&
		   section->sampling_period_us == -1) {
		section->sampling_period_us =
			SAMPLING_MS_DEFAULT * 1000;
		dbgmsg("Using default sampling period %dms "
		       "for sensor[%s]\n", SAMPLING_MS_DEFAULT,
		       section->desc);
	}
}

void validate_config(thermal_setting_t *settings)
{
	sensor_setting_t *s;
	int i, j, k;

	for (i = 0; i < SENSOR_IDX_MAX; i++) {
		s = &settings->sensors[i];

		/* Skip validation for unconfigured sensors */
		if (s->desc == NULL)
			continue;

		/* Fill out the number of thresholds based on min entry */
		s->num_thresholds = s->_n_thresholds;
		if (s->num_thresholds > s->_n_to_clear)
			s->num_thresholds = s->_n_to_clear;
		if (s->num_thresholds > s->_n_actions)
			s->num_thresholds = s->_n_actions;
		if (s->num_thresholds > s->_n_action_info)
			s->num_thresholds = s->_n_action_info;

		/* Ensure thresolds are in ascending order */
                /* For BCL, thresholds are in descending order */
		if (s->num_thresholds > 1) {
			if (strncmp(s->desc, "bcl", strlen("bcl")) != 0)
				qsort(s->t, s->num_thresholds, sizeof(threshold_t),
					threshold_array_compare);
			else
				qsort(s->t, s->num_thresholds, sizeof(threshold_t),
					bcl_threshold_array_compare);
		}

		/* Sort so as to move shutdown to last action */
		for (j = 0; j < s->num_thresholds; j++) {
			for (k = 0; k < s->t[j].num_actions; k++) {
				if (s->t[j].actions[k].action == SHUTDOWN &&
						k < s->t[j].num_actions - 1) {
					int value = 0;

					value = s->t[j].actions[k].info;
					s->t[j].actions[k].action = s->t[j].actions[k+1].action;
					s->t[j].actions[k].info = s->t[j].actions[k+1].info;
					s->t[j].actions[k+1].action= SHUTDOWN;
					s->t[j].actions[k+1].info = value;
				}
			}
		}

		/* Fallback on default sampling times, if not configured */
		fallback_sampling_values(settings, s);
		for (j = 0; j < s->num_thresholds; j++) {
			for (k = 0; k < s->t[j].num_actions; k++)
				dbgmsg(" %d %d %s %d\n",
				       s->t[j].lvl_trig,
				       s->t[j].lvl_clr,
				       action_names[s->t[j].actions[k].action],
				       s->t[j].actions[k].info);
		}
	}
}

int load_config(thermal_setting_t *settings, const char *pFName)
{
	int config_fd;

	/* reset to default before loading new configurationn */
	init_settings(settings);

	if (pFName) {
		config_fd = open(pFName, O_RDONLY);
		if (config_fd >= 0) {
			if (!parse_config(settings, config_fd)) {
				update_def_sensor_settings(settings);
			}
		}
		else {
			update_def_sensor_settings(settings);
		}
		close(config_fd);
	}
	else {
		update_def_sensor_settings(settings);
	}
	validate_config(settings);
	return 1;
}

