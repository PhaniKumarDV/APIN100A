/*
 * Copyright (c) 2016 Qualcomm Atheros, Inc..
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "includes.h"
#include "common.h"
#include "linux_ioctl.h"
#include "ieee802_11_defs.h"
#include "linux_wext.h"
#include "eloop.h"
#include "netlink.h"
#include "priv_netlink.h"
#include "pthread.h"

#define _BYTE_ORDER _BIG_ENDIAN
#include "ieee80211_external.h"

#define SON_ACTION_BUF_SIZE 256
#define WPS_STATE_INTERVAL 120
#define WPS_SON_OUI1 0x8c   /* to be set by orbi with big endian orbi_oui */
#define WPS_SON_OUI2 0xfd
#define WPS_SON_OUI3 0xf0

static int wps_state;

char *ap_ifname = NULL;
char *sta_ifname = NULL;
char *ap_radio = NULL;
char *sta_radio = NULL;
char *ap_macaddr = NULL;
char *sta_macaddr = NULL;

char ap_macaddr_hex[20];

char *nbh_apiface[16];
char *nbh_radio[16];
char *nbh_macaddr[20];
static int no_nbh_vaps;

static int is_re;
static int is_root;
static int wps_propagate;
static int inform_root_propagate;
static int wps_probe_req_seen;

struct son_driver_data {
    int ioctl_sock;
    struct netlink_data *netlink;
    char *wifi_ifname;
    s32 pipeFd;
};

struct son_action_mgt_args {
    u_int8_t    category;
    u_int8_t    action;
};

union son_event_data {
    struct son_rx_mgmt {
        const u8 *frame;
        size_t frame_len;
    } son_rx_mgmt;
};

enum son_event_type {
    WPSIE_LISTEN_RE,
    WPS_INFORM_ROOT,
    START_WPS,
    DISABLE_WPS,
    STOP_WPS
};

struct ieee80211_action_vs_public {
    u_int8_t    vendor_oui[3];         /* 0x8c, 0xfd, 0xf0 for QCA */
    u_int8_t    vs_public_action;   /* type of vendor specific variable length content */
    /* followed by vendor specific variable length content */
}__packed;

#define WPS_LISTEN_EV 0x01 // Generated when button is pushed at the root
#define ROOT_INFORMED_EV 0x02
#define RE_START_WPS_EV 0x03
#define DISABLE_WPS_EV 0x04
#define STOP_WPS_EV 0x05
#define WPS_SON_PIPE_PATH "/var/run/sonwps.pipe"

void pbc_sonwps_get_pipemsg(s32 fd, void *eloop_ctx, void *sock_ctx);

static int is_re;
static int is_root;

void handle_wps_state_timer(void *eloop_ctx, void *timeout_ctx)
{
    wps_state = 0;
    printf("wps timeout: wps_state set to 0\n");
}

static int re_macaddr_match(char *macaddr)
{
   int i;
   for (i = 0; i < IEEE80211_ADDR_LEN; i++)
   {
       if( (u8)macaddr[i] != (u8)ap_macaddr_hex[i])
           return 0;
   }
   return 1;
}

static int char2hex(char *charstr)
{
    int i ;
    int hex_len, len = strlen(charstr);

    for(i=0; i<len; i++) {
        /* check 0~9, A~F */
        charstr[i] = ((charstr[i]-48) < 10) ? (charstr[i] - 48) : (charstr[i] - 55);
        /* check a~f */
        if ( charstr[i] >= 42 )
            charstr[i] -= 32;
        if ( charstr[i] > 0xf )
            return -1;
    }

    /* hex len is half the string len */
    hex_len = len /2 ;
    if (hex_len *2 < len)
        hex_len ++;

    for(i=0; i<hex_len; i++)
        charstr[i] = (charstr[(i<<1)] << 4) + charstr[(i<<1)+1];

    return 0;
}

static const char * athr_get_ioctl_name(int op)
{
    switch (op) {
        case IEEE80211_IOCTL_SEND_MGMT:
                return "SEND_MGMT";
        case IEEE80211_IOCTL_FILTERFRAME:
                return "FILTER_TYPE";
        default:
                return "??";
    }
}

static int stop_wps_on_nbh_ap(char *vap, char *radio)
{
    char name[50];
    struct dirent *d_ent;
    DIR *d;
    pid_t pid;
    int status;

    name[0] = '\0';
    snprintf(name, sizeof(name), "/var/run/hostapd-%s", radio);
    printf("opening directory %s\n", name);
    d = opendir(name);
    if (!d) {
        printf("can't open directory %s\n", name);
        return -1;
    }

    name[0] = '\0';
    d_ent = readdir(d);
    while (d_ent) {
        if (!strncmp(d_ent->d_name, vap, IFNAMSIZ)) {
            snprintf(name, sizeof(name), "/var/run/hostapd-%s", radio);
            break;
        }
        d_ent = readdir(d);
    }
    closedir(d);

    if (name[0] != '\0') {
        char *const args[] = {
                   "hostapd_cli", "-i", vap, "-p", name, "wps_cancel", NULL };

        pid = fork();

        switch (pid) {
        case 0: // Child
            // if (execve("/usr/sbin/hostapd_cli", args, NULL) < 0) {}
            if (execvp("/usr/sbin/hostapd_cli", args) < 0) {
                printf("execvp failed (errno "
                        "%d %s)\n", errno, strerror(errno));
            }

            /* Shouldn't come here if execvp succeeds */
            exit(EXIT_FAILURE);

        case -1:
            printf("fork failed (errno "
                    "%d %s)\n", errno, strerror(errno));
            return -1;

        default: // Parent
            if (wait(&status) < 0) {
                printf("wait failed (errno "
                        "%d %s)\n", errno, strerror(errno));
                return -1;
            }

            if (!WIFEXITED(status)
                || WEXITSTATUS(status) == EXIT_FAILURE) {
                printf("child exited with error %d\n",
                !WIFEXITED(status) ? -1 : WEXITSTATUS(status));
                return -1;
            }

            return 0;
        }
    } else {
        printf("file %s not found\n", vap);
        return -1;
    }
}

static int start_wps_on_ap()
{
    char name[50];
    struct dirent *d_ent;
    DIR *d;
    pid_t pid;
    int status;

    name[0] = '\0';
    snprintf(name, sizeof(name), "/var/run/hostapd-%s", ap_radio);
    printf("opening directory %s\n", name);
    d = opendir(name);
    if (!d) {
        printf("can't open directory %s\n", name);
        return -1;
    }

    name[0] = '\0';
    d_ent = readdir(d);
    while (d_ent) {
        if (!strncmp(d_ent->d_name, ap_ifname, IFNAMSIZ)) {
            snprintf(name, sizeof(name), "/var/run/hostapd-%s", ap_radio);
            break;
        }
        d_ent = readdir(d);
    }
    closedir(d);

    if (name[0] != '\0') {
        char *const args[] = {
                   "hostapd_cli", "-i", ap_ifname, "-p", name, "wps_pbc", NULL };

        pid = fork();

        switch (pid) {
        case 0: // Child
            if (execvp("/usr/sbin/hostapd_cli", args) < 0) {
                printf("exexecvp failed (errno "
                        "%d %s)\n", errno, strerror(errno));
            }
            /* Shouldn't come here if execvp succeeds */
            exit(EXIT_FAILURE);

        case -1:
            printf("fork failed (errno "
                    "%d %s)\n", errno, strerror(errno));
            return -1;

        default: // Parent
            if (wait(&status) < 0) {
                printf("wait failed (errno "
                        "%d %s)\n", errno, strerror(errno));
                return -1;
            }

            if (!WIFEXITED(status)
                || WEXITSTATUS(status) == EXIT_FAILURE) {
                printf("child exited with error %d\n",
                !WIFEXITED(status) ? -1 : WEXITSTATUS(status));
                return -1;
            }

            return 0;
        }
    } else {
        printf("file %s not found\n", ap_ifname);
        return -1;
    }
}

static int
set80211priv(struct son_driver_data *drv, int op, void *data, int len)
{
    struct iwreq iwr;
    int do_inline = len < IFNAMSIZ;

    /* Certain ioctls must use the non-inlined method */
    if ( op == IEEE80211_IOCTL_FILTERFRAME)
    {
        do_inline = 0;
    }

    memset(&iwr, 0, sizeof(iwr));
    if(is_root)
    {
        os_strlcpy(iwr.ifr_name, ap_ifname, IFNAMSIZ);
    }

    if(is_re)
    {
        os_strlcpy(iwr.ifr_name, sta_ifname, IFNAMSIZ);
    }

    if(is_re && wps_propagate)
    {
        os_strlcpy(iwr.ifr_name, ap_ifname, IFNAMSIZ);
    }

    if (do_inline)
    {
        /*
        * Argument data fits inline; put it there.
        */
        memcpy(iwr.u.name, data, len);
    }
    else
    {
        /*
        * Argument data too big for inline transfer; setup a
        * parameter block instead; the kernel will transfer
        * the data for the driver.
        */
        iwr.u.data.pointer = data;
        iwr.u.data.length = len;
    }
    wps_propagate = 0;

    if (ioctl(drv->ioctl_sock, op, &iwr) < 0)
    {
        printf("son: %s: %s: ioctl op=0x%x "
                           "(%s) len=%d failed: %d (%s)",
                           __func__, iwr.ifr_name, op,
                           athr_get_ioctl_name(op),
                           len, errno, strerror(errno));
        return -1;
    }
    return 0;
}



static int son_tx_wpspbc_action(
    struct son_driver_data *drv,
    struct son_action_mgt_args *actionargs,
    const u8 *frm)
{
    u8 buf[256];
    struct ieee80211_mgmt *mgmt;
    u8 *pos;
    size_t length;
    struct ieee80211req_mgmtbuf *mgmt_frm;
    struct ieee80211_action_vs_public ven;

    mgmt = (struct ieee80211_mgmt *) frm;
    mgmt_frm = (struct ieee80211req_mgmtbuf *) buf;
    memset(&buf, 0, sizeof(buf));
    mgmt->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
                                           (WLAN_FC_STYPE_ACTION));
    memcpy(mgmt_frm->macaddr, (u8 *)mgmt->sa, IEEE80211_ADDR_LEN);
    if(actionargs->category ==  WLAN_ACTION_VENDOR_SPECIFIC) {
        mgmt->u.action.category = 0x7f;//WLAN_ACTION_VENDOR_SPECIFIC;
        ven.vendor_oui[0] = WPS_SON_OUI1;
        ven.vendor_oui[1] = WPS_SON_OUI2;
        ven.vendor_oui[2] = WPS_SON_OUI3;
        ven.vs_public_action = WLAN_EID_VENDOR_SPECIFIC;
        switch(actionargs->action) {
            case WPSIE_LISTEN_RE:
                memcpy(mgmt_frm->macaddr, broadcast_ether_addr, IEEE80211_ADDR_LEN);
                pos = (u8*)&mgmt->u.action.u.vs_public_action;
                memcpy(pos, (u8*) &ven, 4);
                pos = mgmt->u.action.u.vs_public_action.variable;
                mgmt->u.action.u.vs_public_action.variable[0] = 0x07;
                pos += 1;
                memcpy(pos, (u8*) &ven.vendor_oui, 3);
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[4] = 0x0d;
                mgmt->u.action.u.vs_public_action.variable[5] = 0x01; //WPSIE_LISTEN_RE;
                mgmt->u.action.u.vs_public_action.variable[6] = 0x00;
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[7] = 0x00;
                pos +=1;
                length = pos - (u8*) &mgmt->u.action.category;
                mgmt_frm->buflen = length + 24;

                if (mgmt_frm->buflen > sizeof(buf))
                {
                    printf("frame too long\n");
                    return -1;
                }
                // TODO - Any SON way of finding out the addresses of REs
                break;

            case WPS_INFORM_ROOT:
                pos = (u8*)&mgmt->u.action.u.vs_public_action;
                memcpy(pos, (u8*) &ven, 4);
                pos = mgmt->u.action.u.vs_public_action.variable;
                mgmt->u.action.u.vs_public_action.variable[0] = 0x0c;
                pos += 1;
                memcpy(pos, (u8*) &ven.vendor_oui, 3);
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[4] = 0x0d;
                mgmt->u.action.u.vs_public_action.variable[5] = 0x02; //WPS_INFORM_ROOT;
                mgmt->u.action.u.vs_public_action.variable[6] = 0x00;
                pos += 3;
                if (inform_root_propagate == 1)
                {
                    memcpy(pos, (u8 *)(mgmt->u.action.u.vs_public_action.variable+7), IEEE80211_ADDR_LEN);
                    inform_root_propagate = 0;
                }
                else
                {
                    memcpy(pos, (u8 *)ap_macaddr_hex, IEEE80211_ADDR_LEN);
                }
                pos += IEEE80211_ADDR_LEN;
                length = pos - (u8*) &mgmt->u.action.category;
                mgmt_frm->buflen = length + 24;

                if (&mgmt_frm->buf[0] + length > buf + sizeof(buf))
                {
                    printf("frame too long\n");
                    return -1;
                }
                break;

            case START_WPS:
                if (wps_propagate == 1)
                {
                    memcpy(mgmt_frm->macaddr, broadcast_ether_addr, IEEE80211_ADDR_LEN);
                }
                pos = (u8*)&mgmt->u.action.u.vs_public_action;
                memcpy(pos, (u8*) &ven, 4);
                pos = mgmt->u.action.u.vs_public_action.variable;
                mgmt->u.action.u.vs_public_action.variable[0] = 0x0c;
                pos += 1;
                memcpy(pos, (u8*) &ven.vendor_oui, 3);
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[4] = 0x0d;
                mgmt->u.action.u.vs_public_action.variable[5] = 0x03;
                mgmt->u.action.u.vs_public_action.variable[6] = 0x00;
                pos += 3;
                memcpy(pos, (u8 *)(mgmt->u.action.u.vs_public_action.variable+7), IEEE80211_ADDR_LEN);
                pos += IEEE80211_ADDR_LEN;
                length = pos - (u8*) &mgmt->u.action.category;
                mgmt_frm->buflen = length + 24;

                if (&mgmt_frm->buf[0] + length > buf + sizeof(buf))
                {
                    printf("frame too long\n");
                    return -1;
                }
                break;

            case STOP_WPS:
                pos = (u8*)&mgmt->u.action.u.vs_public_action;
                memcpy(pos, (u8*) &ven, 4);
                pos = mgmt->u.action.u.vs_public_action.variable;
                mgmt->u.action.u.vs_public_action.variable[0] = 0x07;
                pos += 1;
                memcpy(pos, (u8*) &ven.vendor_oui, 3);
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[4] = 0x0d;
                mgmt->u.action.u.vs_public_action.variable[5] = 0x05;
                mgmt->u.action.u.vs_public_action.variable[6] = 0x00;
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[7] = 0x00;
                pos +=1;
                length = pos - (u8*) &mgmt->u.action.category;
                mgmt_frm->buflen = length + 24;

                if (mgmt_frm->buflen > sizeof(buf))
                {
                    printf("frame too long\n");
                    return -1;
                }
                break;

            case DISABLE_WPS:
                memcpy(mgmt_frm->macaddr, broadcast_ether_addr, IEEE80211_ADDR_LEN);
                pos = (u8*)&mgmt->u.action.u.vs_public_action;
                memcpy(pos, (u8*) &ven, 4);
                pos = mgmt->u.action.u.vs_public_action.variable;
                mgmt->u.action.u.vs_public_action.variable[0] = 0x07;
                pos += 1;
                memcpy(pos, (u8*) &ven.vendor_oui, 3);
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[4] = 0x0d;
                mgmt->u.action.u.vs_public_action.variable[5] = 0x04; //DISABLE_WPS_EV;
                mgmt->u.action.u.vs_public_action.variable[6] = 0x00;
                pos += 3;
                mgmt->u.action.u.vs_public_action.variable[7] = 0x00;
                pos +=1;
                length = pos - (u8*) &mgmt->u.action.category;
                mgmt_frm->buflen = length + 24;

                if (mgmt_frm->buflen > sizeof(buf))
                {
                    printf("frame too long\n");
                    return -1;
                }
                break;

            default:
                return -1;
                break;
        }
        memcpy(&mgmt_frm->buf[0], mgmt, mgmt_frm->buflen);
        return set80211priv(drv, IEEE80211_IOCTL_SEND_MGMT, mgmt_frm, mgmt_frm->buflen);
    }
    else
        return -1;
}

static void son_send_action(void *ctx, enum son_event_type event,
                          union son_event_data *data)
{
    struct son_driver_data *drv = ctx;

    switch (event) {
        case WPSIE_LISTEN_RE:
        {
            struct son_action_mgt_args act_args;
            act_args.category = WLAN_ACTION_VENDOR_SPECIFIC;
            act_args.action   = WPSIE_LISTEN_RE;

            son_tx_wpspbc_action(drv, &act_args, data->son_rx_mgmt.frame);
        }
            break;
        case WPS_INFORM_ROOT:
        {
            struct son_action_mgt_args act_args;
            act_args.category = WLAN_ACTION_VENDOR_SPECIFIC;
            act_args.action   = WPS_INFORM_ROOT;

            son_tx_wpspbc_action(drv, &act_args, data->son_rx_mgmt.frame);
        }
            break;
        case START_WPS:
        {
            struct son_action_mgt_args act_args;
            act_args.category = WLAN_ACTION_VENDOR_SPECIFIC;
            act_args.action   = START_WPS;

            son_tx_wpspbc_action(drv, &act_args, data->son_rx_mgmt.frame);
        }
             break;
        case DISABLE_WPS://broadcast disable
        {
            struct son_action_mgt_args act_args;
            act_args.category = WLAN_ACTION_VENDOR_SPECIFIC;
            act_args.action   = DISABLE_WPS;

            son_tx_wpspbc_action(drv, &act_args, data->son_rx_mgmt.frame);
        }
            break;
        case STOP_WPS://RE specific disable
        {
            struct son_action_mgt_args act_args;
            act_args.category = WLAN_ACTION_VENDOR_SPECIFIC;
            act_args.action   = STOP_WPS;

            son_tx_wpspbc_action(drv, &act_args, data->son_rx_mgmt.frame);
        }
            break;
        default:
            printf("Unknown event %d", event);
            break;
    }
}

static void son_raw_receive(void *ctx, const u8 *src_addr, const u8 *buf,
                                size_t len)
{
    struct son_driver_data *drv = ctx;
    const struct ieee80211_mgmt *mgmt;
    union son_event_data event;
    u16 fc, stype;

    if (len < IEEE80211_HDRLEN)
        return;

    mgmt = (const struct ieee80211_mgmt *) buf;

    fc = le_to_host16(mgmt->frame_control);

    if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT)
        return;

    stype = WLAN_FC_GET_STYPE(fc);

    switch (stype) {
        case WLAN_FC_STYPE_PROBE_REQ:      //when wps_ie seen in probe request
            os_memset(&event, 0, sizeof(event));
            event.son_rx_mgmt.frame = buf;
            event.son_rx_mgmt.frame_len = len;
            if(wps_state == 1 && is_root)
            {
                int i;

                //action frame to REs to disable listen
                son_send_action(drv, DISABLE_WPS, &event);
                printf("disable wps listen and start wps on root\n");
                start_wps_on_ap();
                wps_state = 0;
                for(i = 0; i < no_nbh_vaps; i++)
                {
                    stop_wps_on_nbh_ap(nbh_apiface[i], nbh_radio[i]);
                }
            }
            else if(wps_state == 1 && wps_probe_req_seen == 0)
            {
                printf("inform root of wps prob_req\n");
                son_send_action(drv, WPS_INFORM_ROOT, &event); //ACTION 1
                wps_probe_req_seen = 1;
            }
            break;

        case WLAN_FC_STYPE_ACTION:
            os_memset(&event, 0, sizeof(event));
            event.son_rx_mgmt.frame = buf;
            event.son_rx_mgmt.frame_len = len;

            switch(mgmt->u.action.u.vs_public_action.variable[5]) {
                //root to RE to start listening
                case WPS_LISTEN_EV:
                {
                    wps_state = 1;
                    wps_probe_req_seen = 0;
                    printf("wps_state is 1, probe not seen!\n");
                    eloop_cancel_timeout(handle_wps_state_timer, NULL, NULL);
                    eloop_register_timeout(WPS_STATE_INTERVAL, 0, handle_wps_state_timer,
                               NULL, NULL);
                    wps_propagate = 1;
                    son_send_action(drv, WPSIE_LISTEN_RE, &event);
                    break;
                }

                case ROOT_INFORMED_EV:
		    if(wps_state == 1 && is_root)
                    {
                        //root to RE to start wps in response to WPS_INFORM_ROOT frame
                        son_send_action(drv, START_WPS, &event);   //ACTION2
                        wps_state = 0;
                        printf("Root send RE start wps action\n");
                        break;
                    }
                    else if (wps_state == 1)
                    {
                        /* propagate action to root. Since, sta_iface
                         * is to be used, wps_propagate is not set */
                        inform_root_propagate = 1;
                        wps_probe_req_seen = 1;
                        son_send_action(drv, WPS_INFORM_ROOT, &event);
                        break;
                    }
                    else if (wps_state == 0)
                    {
                        son_send_action(drv, STOP_WPS, &event);
                        break;
                    }
                    break;

                case RE_START_WPS_EV:
                    if (wps_state == 1)
                    {
                        char macaddr[20];
                        memcpy(macaddr, (u8 *)(mgmt->u.action.u.vs_public_action.variable+7), IEEE80211_ADDR_LEN);

                        if (re_macaddr_match(macaddr))
                        {
                            printf("command to start wps in RE\n");//calls hostapd to start wps in RE
                            start_wps_on_ap();
                            wps_propagate = 1;
                            son_send_action(drv, DISABLE_WPS, &event);
                            wps_state=0;
                        }
                        else
                        {
                            wps_propagate = 1;
                            son_send_action(drv, START_WPS, &event);
                        }
                    }
                    break;

                case STOP_WPS_EV:
                case DISABLE_WPS_EV:
                if (wps_state == 1)
                {
                    printf("wps disabled at RE\n");
                    wps_state = 0;
                    wps_propagate = 1;
                    son_send_action(drv, DISABLE_WPS, &event);
                    break;
                }

                default:
                    break;
            }
        default:
            break;
    }

}

static int sonwps_init_pipefd() {
    int err;
    int fd;

    unlink(WPS_SON_PIPE_PATH);
    err = mkfifo(WPS_SON_PIPE_PATH, 0666);
    if ((err == -1) && (errno != EEXIST)) {
        return -1;
    }

    fd = open(WPS_SON_PIPE_PATH, O_RDWR);
    if (fd == -1) {
        perror("open(pipe)");
        return -1;
    }

        //sonwps_reset_pipefd(drv);
    return fd;
}

int sonwps_reset_pipefd(struct son_driver_data *pData)
{
    if (pData->pipeFd > 0 )
    {
        eloop_unregister_read_sock(pData->pipeFd);
        close (pData->pipeFd);
    }

    pData->pipeFd = sonwps_init_pipefd();
    if (pData->pipeFd < 0) {
        printf("InitPipe");
        return -1;
    }
    printf("Reset pipe FD: %d\n", pData->pipeFd);
    eloop_register_read_sock(pData->pipeFd, pbc_sonwps_get_pipemsg, pData, NULL);

    return 0;
}

void pbc_sonwps_get_pipemsg(s32 fd, void *eloop_ctx, void *sock_ctx)
{
#define MGMT_FRAM_TAG_SIZE 30
    char buf[256];
    char *pos;
    int  len;
    int  duration;
    union son_event_data event;

    struct son_driver_data *drv = (struct son_driver_data*)eloop_ctx;

    len = read(fd, buf, sizeof(buf) -1 );
    if (len <= 0) {
        printf("pbcGetPipeMsgCB - read");
        sonwps_reset_pipefd(drv);
        return;
    }
    buf[len] = '\0';

    printf("Got event: %s\n", buf);
    if (strncmp(buf, "wps_pbc", 7) != 0)
    {
        printf("Reset pipe beacuse of Unknown event: %s\n", buf);
        sonwps_reset_pipefd(drv);
        return;
    }

    printf("PUSH BUTTON EVENT!\n");
    pos = buf + 7;
    duration = atoi(pos);
    printf("got duration: %d\n", duration);
    if (duration < 0)
        duration = 0;
    printf("wps activated\n");

    if(is_root)
    {
        wps_state = 1 ;
        printf("wps_state is 1!\n");
        eloop_cancel_timeout(handle_wps_state_timer, NULL, NULL);
        eloop_register_timeout(WPS_STATE_INTERVAL, 0, handle_wps_state_timer,
                               NULL, NULL);

        event.son_rx_mgmt.frame = (u8 *)buf + MGMT_FRAM_TAG_SIZE;
        son_send_action(drv, WPSIE_LISTEN_RE, &event);
    }

    return;
}

static void
son_wireless_event_wireless_custom(struct son_driver_data *drv,
                                       char *custom, char *end)
{
#define MGMT_FRAM_TAG_SIZE 30 /* hardcoded in driver */

    if (strncmp(custom, "Manage.prob_req_wps ", 20) == 0)
    {
        int len = atoi(custom + 20);
        printf("son: wps push button seen in prob_req\n");
        if (len < 0 || custom + MGMT_FRAM_TAG_SIZE + len > end)
        {
            printf("Invalid Manage.prob_req event length %d", len);
            return;
        }
        son_raw_receive(drv, NULL, (u8 *) custom + MGMT_FRAM_TAG_SIZE, len);
    }
    else if (strncmp(custom, "Manage.action ", 14) == 0)
    {
        /* Format: "Manage.assoc_req <frame len>" | zero padding | frame
        */
        int len = atoi(custom + 14);
        if (len < 0 || custom + MGMT_FRAM_TAG_SIZE + len > end)
        {
            printf("Invalid Manage.action event length %d", len);
            return;
        }
        son_raw_receive(drv, NULL, (u8 *) custom + MGMT_FRAM_TAG_SIZE, len);
    }
}


static void
son_wireless_event_wireless(struct son_driver_data *drv,
                                char *data, int len)
{
    struct iw_event iwe_buf, *iwe = &iwe_buf;
    char *pos, *end, *custom, *buf;

    pos = data;
    end = data + len;

    while (pos + IW_EV_LCP_LEN <= end)
    {
        /* Event data may be unaligned, so make a local, aligned copy
                 * before processing. */
        memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
        if (iwe->len <= IW_EV_LCP_LEN)
            return;

        custom = pos + IW_EV_POINT_LEN;
        if ((iwe->cmd == IWEVMICHAELMICFAILURE || iwe->cmd == IWEVASSOCREQIE ||
                     iwe->cmd == IWEVCUSTOM)) {
            /* WE-19 removed the pointer from struct iw_point */
            char *dpos = (char *) &iwe_buf.u.data.length;
            int dlen = dpos - (char *) &iwe_buf;
            memcpy(dpos, pos + IW_EV_LCP_LEN, sizeof(struct iw_event) - dlen);
        }
        else
        {
            memcpy(&iwe_buf, pos, sizeof(struct iw_event));
            custom += IW_EV_POINT_OFF;
        }

        switch (iwe->cmd) {
            case IWEVREGISTERED:
                break;
            case IWEVASSOCREQIE:
            case IWEVCUSTOM:
                if (custom + iwe->u.data.length > end)
                    return;
                buf = malloc(iwe->u.data.length + 1);
                if (buf == NULL)
                    return;         /* XXX */
                memcpy(buf, custom, iwe->u.data.length);
                buf[iwe->u.data.length] = '\0';
                son_wireless_event_wireless_custom(drv, buf, buf + iwe->u.data.length);
                free(buf);
                break;
        }

        pos += iwe->len;
    }
}



static void
son_wireless_event_rtm_newlink(void *ctx,
                                   struct ifinfomsg *ifi, u8 *buf, size_t len)
{
    struct son_driver_data *drv = ctx ;
    int attrlen, rta_len;
    struct rtattr *attr;


    attrlen = len;
    attr = (struct rtattr *) buf;

    rta_len = RTA_ALIGN(sizeof(struct rtattr));
    while (RTA_OK(attr, attrlen))
    {
        if (attr->rta_type == IFLA_WIRELESS)
        {
            son_wireless_event_wireless(drv, ((char *) attr) + rta_len, attr->rta_len - rta_len);
        }
        attr = RTA_NEXT(attr, attrlen);
    }
}



void *son_main_function(void *conf_file)
{
    struct son_driver_data *drv ;
    struct netlink_config *cfg;
    struct ieee80211req_set_filter filt;
    const char *son_conf = (const char *)conf_file;
    FILE *file;
    int nbh_vaps = 0;

    drv=  os_zalloc(sizeof(*drv));

    if(!drv)
        goto out;

    drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (drv->ioctl_sock < 0)
    {
        goto out;
    }

    if (eloop_init())
    {
        printf("Failed to initialize event loop");
        goto out;
    }

    cfg = os_zalloc(sizeof(*cfg));
    if (cfg == NULL)
        goto out;
    cfg->ctx = drv;
    cfg->newlink_cb = son_wireless_event_rtm_newlink;
    drv->netlink = netlink_init(cfg);

    if (drv->netlink == NULL)
    {
        printf("netlink init Error \n");
        os_free(cfg);
        goto out;
    }
    drv->pipeFd = sonwps_init_pipefd();
    if (drv->pipeFd < 0) {
        perror("InitPipe");
        goto out;
    }
    eloop_register_read_sock(drv->pipeFd, pbc_sonwps_get_pipemsg, drv, NULL);

    file = fopen(son_conf, "r");

    if ( file == 0 )
    {
        printf( "Could not open son config file\n" );
        goto out;
    }
    else
    {
        int x;
        int i = 0;
        char mode[8];
        char ifname[8];
        char radio[8];
        char macaddr[20];
        while ((x = fgetc(file)) != EOF)
        {
            fscanf(file, "%s %s %s %s", mode, ifname, radio, macaddr);
            if(strcmp(mode,"sta") == 0)
            {
                sta_ifname = malloc(sizeof(ifname));
                if (sta_ifname == NULL)
                    goto out;
                strncpy(sta_ifname, ifname, sizeof(ifname));
                sta_radio = malloc(sizeof(radio));
                if (sta_radio == NULL)
                    goto out;
                strncpy(sta_radio, radio, sizeof(radio));
                sta_macaddr = malloc(sizeof(macaddr));
                if (sta_macaddr == NULL)
                    goto out;
                strncpy(sta_macaddr, macaddr, sizeof(macaddr));
            }
            if(strcmp(mode,"ap") == 0)
            {
                ap_ifname = malloc(sizeof(ifname));
                if (ap_ifname == NULL)
                    goto out;
                strncpy(ap_ifname, ifname, sizeof(ifname));
                ap_radio = malloc(sizeof(radio));
                if (ap_radio == NULL)
                    goto out;
                strncpy(ap_radio, radio, sizeof(radio));
                ap_macaddr = malloc(sizeof(macaddr));
                if (ap_macaddr == NULL)
                    goto out;
                strncpy(ap_macaddr, macaddr, sizeof(macaddr));
                strncpy(ap_macaddr_hex, macaddr, 2);
                strncat(ap_macaddr_hex+2, macaddr+3, 2);
                strncat(ap_macaddr_hex+4, macaddr+6, 2);
                strncat(ap_macaddr_hex+6, macaddr+9, 2);
                strncat(ap_macaddr_hex+8, macaddr+12, 2);
                strncat(ap_macaddr_hex+10, macaddr+15, 2);
                char2hex(ap_macaddr_hex);
            }
            if(strcmp(mode,"nbh_ap") == 0)
            {
                nbh_apiface[i] = malloc(sizeof(ifname));
                if (nbh_apiface[i] == NULL)
                    goto out;
                strncpy(nbh_apiface[i], ifname, sizeof(ifname));
                nbh_radio[i] = malloc(sizeof(radio));
                if (nbh_radio[i] == NULL)
                    goto out;
                strncpy(nbh_radio[i], radio, sizeof(radio));
                nbh_macaddr[i] = malloc(sizeof(macaddr));
                if (nbh_macaddr[i] == NULL)
                    goto out;
                strncpy(nbh_macaddr[i], macaddr, sizeof(macaddr));
                i++;
            }
            no_nbh_vaps = i;
        }
        fclose(file);
        /* TODO - Does ap_ifname matter here ? */
        if (sta_ifname != NULL && ap_ifname != NULL)
            is_re = 1;
        if (sta_ifname == NULL && ap_ifname != NULL)
            is_root = 1;
    }

    if (is_re == 0 && is_root == 0)
        goto out;

    if (is_re) {
        filt.app_filterype = IEEE80211_FILTER_TYPE_ACTION;
        if (filt.app_filterype)
        {
            set80211priv(drv, IEEE80211_IOCTL_FILTERFRAME, &filt, sizeof(struct ieee80211req_set_filter));
        }
    }

    eloop_run();

    os_free(drv);
    free(sta_ifname);
    free(ap_ifname);
    free(ap_radio);
    free(sta_radio);
    free(ap_macaddr);
    free(sta_macaddr);
    for(nbh_vaps = 0; nbh_vaps < no_nbh_vaps; nbh_vaps++)
        {
            free(nbh_radio[nbh_vaps]);
        }

out:
    printf("son: Exiting thread %s\n", __func__);
    pthread_exit(NULL);
}
