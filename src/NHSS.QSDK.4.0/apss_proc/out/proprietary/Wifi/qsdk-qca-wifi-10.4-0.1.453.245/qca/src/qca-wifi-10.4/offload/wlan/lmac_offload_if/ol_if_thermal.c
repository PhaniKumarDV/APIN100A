/*
* Copyright (c) 2014 Qualcomm Atheros, Inc..
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

#include "qdf_mem.h"
#include "ol_if_athvar.h"
#include "ol_if_thermal.h"

static uint32_t ol_ath_thermal_param_init (struct ol_ath_softc_net80211 *scn)
{
    OS_MEMSET(&(scn->thermal_param.th_stats), 0, sizeof(scn->thermal_param.th_stats));
    OS_MEMSET(&(scn->thermal_param.th_cfg), 0, sizeof(scn->thermal_param.th_cfg));
    OS_MEMSET(&(scn->thermal_param.tmd_cfg), 0, sizeof(scn->thermal_param.tmd_cfg));
    scn->thermal_param.th_cfg.enable = get_tt_supported(scn);
    scn->thermal_param.th_cfg.dc = THERMAL_CONFIG_DEFAULT_DUTY_CYCLE;
    scn->thermal_param.th_cfg.dc_per_event = THERMAL_WMI_EVENT_DC;
    scn->thermal_param.th_cfg.log_lvl = TH_DEBUG_LVL1;

    /*
     *    Level    tmplwm (C)       tmphwm (C)       dcoffpercent (%)
     *      0         NIL              90                 0
     *      1         70               110                50
     *      2         90               125                70
     *      3         110              MAX                100
     */

    /* thermal level / zone 0 config */
    scn->thermal_param.th_cfg.levelconf[0].tmplwm = THERMAL_CONFIG_TMPLWM0;
    scn->thermal_param.th_cfg.levelconf[0].tmphwm = THERMAL_CONFIG_TMPHWM0; /* Celsius */
    scn->thermal_param.th_cfg.levelconf[0].dcoffpercent = THERMAL_CONFIG_DCOFF0;   /* milli seconds */
    scn->thermal_param.th_cfg.levelconf[0].priority = THERMAL_ALL_UNICAST_DATA_QUEUES;
    scn->thermal_param.th_cfg.levelconf[0].policy = THERMAL_POLICY_QUEUE_PAUSE;
    /* thermal level / zone 1 config */
    scn->thermal_param.th_cfg.levelconf[1].tmplwm = THERMAL_CONFIG_TMPLWM1;
    scn->thermal_param.th_cfg.levelconf[1].tmphwm = THERMAL_CONFIG_TMPHWM1; /* Celsius */
    scn->thermal_param.th_cfg.levelconf[1].dcoffpercent = THERMAL_CONFIG_DCOFF1;    /* milli seconds */
    scn->thermal_param.th_cfg.levelconf[1].priority = THERMAL_ALL_UNICAST_DATA_QUEUES;
    scn->thermal_param.th_cfg.levelconf[1].policy = THERMAL_POLICY_QUEUE_PAUSE;
    /* thermal level / zone 2 config */
    scn->thermal_param.th_cfg.levelconf[2].tmplwm = THERMAL_CONFIG_TMPLWM2;
    scn->thermal_param.th_cfg.levelconf[2].tmphwm = THERMAL_CONFIG_TMPHWM2; /* Celsius */
    scn->thermal_param.th_cfg.levelconf[2].dcoffpercent = THERMAL_CONFIG_DCOFF2;    /* milli seconds */
    scn->thermal_param.th_cfg.levelconf[2].priority = THERMAL_ALL_UNICAST_DATA_QUEUES;
    scn->thermal_param.th_cfg.levelconf[2].policy = THERMAL_POLICY_QUEUE_PAUSE;
    /* thermal level / zone 3 config */
    scn->thermal_param.th_cfg.levelconf[3].tmplwm = THERMAL_CONFIG_TMPLWM3;
    scn->thermal_param.th_cfg.levelconf[3].tmphwm = THERMAL_CONFIG_TMPHWM3; /* Celsius */
    scn->thermal_param.th_cfg.levelconf[3].dcoffpercent = THERMAL_CONFIG_DCOFF3;    /* milli seconds */
    scn->thermal_param.th_cfg.levelconf[3].priority = THERMAL_ALL_UNICAST_DATA_QUEUES;
    scn->thermal_param.th_cfg.levelconf[3].policy = THERMAL_POLICY_QUEUE_PAUSE;

    return TH_SUCCESS;
}

static int
ol_ath_thermal_stats_event_handler(ol_scn_t scn, u_int8_t *data, u_int16_t datalen)
{
    uint8_t i = 0;

    if (!scn) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"Error: %s: NULL scn\n", __func__);
        return -EINVAL;
    }

    if(wmi_extract_thermal_stats(scn->wmi_handle, data,
                &scn->thermal_param.th_stats.temp,
                &scn->thermal_param.th_stats.level)) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,
                "Error: %s: Failed to extract tt stats\n", __func__);
        return -EINVAL;
    }
    for (i = 0; i < THERMAL_LEVELS; i++) {
        if(wmi_extract_thermal_level_stats(scn->wmi_handle, data, i,
                    &scn->thermal_param.th_stats.levstats[i].levelcount,
                    &scn->thermal_param.th_stats.levstats[i].dccount)) {

            TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,
           "Error: %s: Failed to extract tt levl stats for %d\n", __func__, i);

        }

    }

    TH_DEBUG_PRINT(TH_DEBUG_LVL4, scn, "%s: temp: %d, level: %d\n", __func__,
         scn->thermal_param.th_stats.temp, scn->thermal_param.th_stats.level);
    return TH_SUCCESS;
}


int32_t ol_ath_config_thermal_mitigation_param(struct ol_ath_softc_net80211 *scn)
{
    struct thermal_mitigation_params param;
    int i = 0;

    if (!scn) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"%s: Error invalid scn:\n", __func__);
        return -EINVAL;
    }

    if (!get_tt_supported(scn)) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"%s:TT not supported\n", __FUNCTION__);
        return TH_FAIL;
    }

    qdf_mem_set(&param, sizeof(param), 0);
    param.enable = scn->thermal_param.th_cfg.enable;
    param.dc = scn->thermal_param.th_cfg.dc;
    param.dc_per_event = scn->thermal_param.th_cfg.dc_per_event;
    for (i = 0; i < THERMAL_LEVELS; i++) {
        param.levelconf[i].tmplwm = scn->thermal_param.th_cfg.levelconf[i].tmplwm;
        param.levelconf[i].tmphwm = scn->thermal_param.th_cfg.levelconf[i].tmphwm;
        param.levelconf[i].dcoffpercent = scn->thermal_param.th_cfg.levelconf[i].dcoffpercent;
        param.levelconf[i].priority = scn->thermal_param.th_cfg.levelconf[i].priority;
    }
/*
#if THERMAL_DEBUG
        p_config = cmd;

        TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"setting WMI with: enable: %d, dc: %d, dc_per_event: %d\n",p_config->enable, p_config->dc, p_config->dc_per_event);
        for (i = 0; i < THERMAL_LEVELS; i++) {
            TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"level: %d, low thresold: %d, high thresold: %d, dcoffpercent: %d, queue priority %d\n",
                            i, p_config->levelconf[i].tmplwm, p_config->levelconf[i].tmphwm, p_config->levelconf[i].dcoffpercent,
                            p_config->levelconf[i].prio);
        }
#endif
*/
    return wmi_unified_thermal_mitigation_param_cmd_send(scn->wmi_handle, &param);
}

inline uint32_t get_tt_supported (struct ol_ath_softc_net80211 *scn)
{
    return ((scn->thermal_param.tt_support) ? 1 : 0);
}

/*
 * Register Thermal Mitigation Functionality
 */

int32_t __ol_ath_thermal_mitigation_attach(struct ol_ath_softc_net80211 *scn)
{
    int retval = TH_SUCCESS;

    if (!scn) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"Error: %s: NULL scn\n", __func__);
        return -EINVAL;
    }

    if (get_tt_supported(scn)) {
        ol_ath_thermal_param_init(scn);
        retval = wmi_unified_register_event_handler(scn->wmi_handle, wmi_tt_stats_event_id,
                ol_ath_thermal_stats_event_handler, WMI_RX_UMAC_CTX);
        if (!retval) {
            retval = ol_ath_config_thermal_mitigation_param(scn);
            if (retval) {
                wmi_unified_unregister_event_handler(scn->wmi_handle, wmi_tt_stats_event_id);
            }
        }
    }

    return retval;
}

int32_t __ol_ath_thermal_mitigation_detach(struct ol_ath_softc_net80211 *scn)
{
    int retval = 0;

    if (!scn) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"Error: %s: NULL scn\n", __func__);
        return -EINVAL;
    }

    if (get_tt_supported(scn)) {
        scn->thermal_param.th_cfg.enable = TH_FALSE;
        retval = ol_ath_config_thermal_mitigation_param(scn);
        retval = wmi_unified_unregister_event_handler(scn->wmi_handle, wmi_tt_stats_event_id);
    }

    return retval;
}


int32_t ol_ath_ioctl_set_thermal_handler (struct ol_ath_softc_net80211 *scn, caddr_t *param)
{
    struct th_config_t th_config;
    int error = 0;
    int cfg_changed = TH_FALSE;
    int i = 0;

    if (!scn) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"%s: Error invalid scn:\n", __func__);
        return -EINVAL;
    }

    if (!get_tt_supported(scn)){
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn, "%s: TT not supported in FW\n", __func__);
        return -EINVAL;
    }

    error = __xcopy_from_user(&th_config, param, sizeof(th_config));
    if (error) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"%s: Error in copying data from user space:\n", __func__);
        error = -EFAULT;
    } else {
        if ((th_config.enable != INVALID_THERMAL_PARAM) &&
                (scn->thermal_param.th_cfg.enable != th_config.enable)) {
            scn->thermal_param.th_cfg.enable = th_config.enable;
            cfg_changed = TH_TRUE;
        }

        if ((th_config.dc != INVALID_THERMAL_PARAM) &&
                (scn->thermal_param.th_cfg.dc != th_config.dc)) {
            scn->thermal_param.th_cfg.dc = th_config.dc;
            cfg_changed = TH_TRUE;
        }

        if ((th_config.dc_per_event != INVALID_THERMAL_PARAM) &&
                (scn->thermal_param.th_cfg.dc_per_event != th_config.dc_per_event)) {
            scn->thermal_param.th_cfg.dc_per_event = th_config.dc_per_event;
            cfg_changed = TH_TRUE;
        }

        if (th_config.log_lvl != INVALID_THERMAL_PARAM) {
            scn->thermal_param.th_cfg.log_lvl = th_config.log_lvl | TH_DEBUG_LVL1;
        }

        for (i = 0; i < THERMAL_LEVELS; i++) {

            if ((th_config.levelconf[i].tmplwm != INVALID_THERMAL_PARAM) &&
                    (scn->thermal_param.th_cfg.levelconf[i].tmplwm != th_config.levelconf[i].tmplwm)) {
                scn->thermal_param.th_cfg.levelconf[i].tmplwm = th_config.levelconf[i].tmplwm;
                cfg_changed = TH_TRUE;
            }
            if ((th_config.levelconf[i].tmphwm != INVALID_THERMAL_PARAM) &&
                    (scn->thermal_param.th_cfg.levelconf[i].tmphwm != th_config.levelconf[i].tmphwm)) {
                scn->thermal_param.th_cfg.levelconf[i].tmphwm = th_config.levelconf[i].tmphwm;
                cfg_changed = TH_TRUE;
            }
            if ((th_config.levelconf[i].dcoffpercent != INVALID_THERMAL_PARAM) &&
                    (scn->thermal_param.th_cfg.levelconf[i].dcoffpercent != th_config.levelconf[i].dcoffpercent)) {
                scn->thermal_param.th_cfg.levelconf[i].dcoffpercent = th_config.levelconf[i].dcoffpercent;
                cfg_changed = TH_TRUE;
            }
            if ((th_config.levelconf[i].priority != INVALID_THERMAL_PARAM) &&
                    (scn->thermal_param.th_cfg.levelconf[i].priority != th_config.levelconf[i].priority)) {
                scn->thermal_param.th_cfg.levelconf[i].priority = th_config.levelconf[i].priority;
                cfg_changed = TH_TRUE;
            }
            if ((th_config.levelconf[i].policy != INVALID_THERMAL_PARAM) &&
                    (scn->thermal_param.th_cfg.levelconf[i].policy != th_config.levelconf[i].policy)) {
                scn->thermal_param.th_cfg.levelconf[i].policy = th_config.levelconf[i].policy;
                /* only one policy THERMAL_POLICY_QUEUE_PAUSE is supported as of now */
                /* cfg_changed = TH_TRUE; */
            }
        }
        if (cfg_changed == TH_TRUE) {
            /*
             * send wmi command WMI_TT_SET_CONF_CMDID down to FW.
             */
            error = ol_ath_config_thermal_mitigation_param(scn);
        }
    }
    return error;
}

int32_t ol_ath_ioctl_get_thermal_handler (struct ol_ath_softc_net80211 *scn, caddr_t *param)
{
    struct thermal_param th_param;
    int error = 0;

    if (!get_tt_supported(scn)){
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn, "%s: TT not supported in FW\n", __func__);
        return -EINVAL;
    }
    OS_MEMCPY(&th_param, &scn->thermal_param, sizeof(th_param));
    error = _copy_to_user((caddr_t)param, &th_param, sizeof(th_param));
    if (error) {
        TH_DEBUG_PRINT(TH_DEBUG_LVL0, scn,"%s: Error in copying data to user space:\n", __func__);
        error = -EFAULT;
    }

    return error;
}

/*
 * Definition of methods and sysfs attributes neded to control
 * thermal level of wifi device.
 */

ssize_t wifi_thermal_temp_show (struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t sensor_tempval = 0;
    int len = -1;

    if (!scn) {
        return -EINVAL;
    }
    sensor_tempval = scn->thermal_param.th_stats.temp;
    len = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", sensor_tempval);
    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: sensor_val = %d, len = %d\n", __func__, sensor_tempval, len);
    return len;
}


ssize_t wifi_thermal_mode_store (struct device *dev,
                               struct device_attribute *attr, const char *buf, size_t size)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t mode = INVALID_THERMAL_PARAM;
    int len = -1;
    char cbuf[12] = {'\0',};
    char *enabled = "enabled";
    char *disabled = "disabled";

    if (!scn || (size > 10)) {
        return -EINVAL;
    }
    memcpy(cbuf, buf, size);
    cbuf[size] = '\0';

    if ((strlen(enabled) == size -1) && (!strncmp(cbuf, enabled, size - 1))) {
        mode = THERMAL_MITIGATION_ENABLED;
    } else if ((strlen(disabled) == size -1) && (!strncmp(cbuf, disabled, size - 1))) {
        mode = THERMAL_MITIGATION_DISABLED;
    }
    if (mode != INVALID_THERMAL_PARAM) {
        if (mode != scn->thermal_param.th_cfg.enable) {
                scn->thermal_param.th_cfg.enable = mode;
                (void)ol_ath_config_thermal_mitigation_param(scn);
        }
        len = size;
    } else {
        len = -EINVAL;
    }
    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: cbuf = %s, mode=(0x%x), len = %d, zize = %d\n", __func__, cbuf, mode, len, size);
    return len;
}


ssize_t wifi_thermal_mode_show (struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t enable = 0;
    int len = -1;
    char *enabled = "enabled";
    char *disabled = "disabled";

    if (!scn) {
        return -EINVAL;
    }
    enable = scn->thermal_param.th_cfg.enable;
    len = snprintf(buf, (ssize_t)PAGE_SIZE, "%s\n", (enable ? enabled : disabled));
    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: mode = %s, len = %d\n", __func__, (enable ? enabled : disabled), len);
    return len;
}

static inline int32_t thermal_tmd_config_send (struct ol_ath_softc_net80211 *scn)
{
    uint8_t change = TH_FALSE;
    int32_t retval = 0;

    if (scn->thermal_param.tmd_cfg.cfg_ready == TMD_CONFIG_READY) {
        if (scn->thermal_param.tmd_cfg.dcoffpercent !=
            scn->thermal_param.th_cfg.levelconf[scn->thermal_param.tmd_cfg.level].dcoffpercent) {
            scn->thermal_param.th_cfg.levelconf[scn->thermal_param.tmd_cfg.level].dcoffpercent =
                scn->thermal_param.tmd_cfg.dcoffpercent;
            change = TH_TRUE;
        }
        scn->thermal_param.tmd_cfg.cfg_ready = TH_FALSE;
        if (change) {
            retval = ol_ath_config_thermal_mitigation_param(scn);
        }
    }
    return retval;
}


ssize_t wifi_thermal_dutycycle_store (struct device *dev,
                               struct device_attribute *attr, const char *buf, size_t size)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t dc = INVALID_THERMAL_PARAM;
    int len = -1;
    char cbuf[5] = {'\0',};

    if (!scn || (size > 4)) {
        return -EINVAL;
    }
    memcpy(cbuf, buf, size);
    cbuf[size] = '\0';

    if (sscanf(cbuf, "%d", &dc) > 0) {
        if (dc >= THERMAL_MIN_DUTY_CYCLE && dc <= THERMAL_MAX_DUTY_CYCLE) {
            if (scn->thermal_param.th_cfg.dc != dc) {
                scn->thermal_param.th_cfg.dc = dc;
                (void)ol_ath_config_thermal_mitigation_param(scn);
            }
            len = size;
        }
    } else {
        len = -EINVAL;
    }
    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn, "%s: dc: %d, len: %d\n", __func__, dc, len);
    return len;
}


ssize_t wifi_thermal_dutycycle_show (struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t dc = 0;
    int len = -1;

    if (!scn) {
        return -EINVAL;
    }
    dc = scn->thermal_param.th_cfg.dc;
    len = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", dc);
    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: dc = %d, len = %d\n", __func__, dc, len);
    return len;
}


ssize_t wifi_thermal_thlvl_store (struct device *dev,
                               struct device_attribute *attr, const char *buf, size_t size)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t thlvl = INVALID_THERMAL_PARAM;
    int len = -1;
    char cbuf[5] = {'\0',};

    if (!scn || (size > 4)) {
        return -EINVAL;
    }
    memcpy(cbuf, buf, size);
    cbuf[size] = '\0';

    if (sscanf(cbuf, "%d", &thlvl) > 0) {
        if (thlvl >= 0 && thlvl < THERMAL_LEVELS) {
            scn->thermal_param.tmd_cfg.level = thlvl;
            scn->thermal_param.tmd_cfg.cfg_ready |= TMD_CONFIG_LEVEL;
            (void)thermal_tmd_config_send(scn);
            len = size;
        } else {
            len = -EINVAL;
        }
    } else {
        len = -EINVAL;
    }

    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: thlvl = %d, len = %d\n", __func__, thlvl, len);
    return len;
}


ssize_t wifi_thermal_thlvl_show (struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t lvl = 0;
    int len = -1;

    if (!scn) {
        return -EINVAL;
    }
    lvl = scn->thermal_param.tmd_cfg.level;
    len = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", lvl);
    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: level = %d, len = %d\n", __func__, lvl, len);
    return len;
}

ssize_t wifi_thermal_offpercent_store (struct device *dev,
                               struct device_attribute *attr, const char *buf, size_t size)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t off = INVALID_THERMAL_PARAM;
    int len = -1;
    char cbuf[5] = {'\0',};

    if (!scn || (size > 4)) {
        return -EINVAL;
    }
    memcpy(cbuf, buf, size);
    cbuf[size] = '\0';

    if (sscanf(cbuf, "%d", &off) > 0) {
        if (off >= THERMAL_MIN_OFF_PERCENT && off <= THERMAL_MAX_OFF_PERCENT) {
            scn->thermal_param.tmd_cfg.dcoffpercent = off;
            scn->thermal_param.tmd_cfg.cfg_ready |= TMD_CONFIG_DCOFF_PERCENT;
            (void)thermal_tmd_config_send(scn);
            len = size;
        } else {
           len = -EINVAL;
        }
    } else {
        len = -EINVAL;
    }

    TH_DEBUG_PRINT(TH_DEBUG_LVL2, scn,"%s: off = %d, len = %d\n", __func__, off, len);
    return len;
}


ssize_t wifi_thermal_offpercent_show (struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    struct net_device *net = to_net_dev(dev);
    struct ol_ath_softc_net80211 *scn = ath_netdev_priv(net);
    uint32_t off = 0;
    int len = -1;

    if (!scn) {
        return -EINVAL;
    }
    off = scn->thermal_param.tmd_cfg.dcoffpercent;
    len = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", off);
    TH_DEBUG_PRINT(TH_DEBUG_LVL1, scn,"%s: offpercent = %d, len = %d\n", __func__, off, len);
    return len;
}


