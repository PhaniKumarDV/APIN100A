#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/wireless.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>
#include <errno.h>
#include <time.h>

/*
 * Linux uses __BIG_ENDIAN and __LITTLE_ENDIAN while BSD uses _foo
 * and an explicit _BYTE_ORDER.  Sorry, BSD got there first--define
 * things in the BSD way...
 */
#ifndef _LITTLE_ENDIAN
#define _LITTLE_ENDIAN  1234    /* LSB first: i386, vax */
#endif
#ifndef _BIG_ENDIAN
#define _BIG_ENDIAN     4321    /* MSB first: 68000, ibm, net */
#endif
#include <asm/byteorder.h>
#if defined(__LITTLE_ENDIAN)
#define _BYTE_ORDER     _LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN)
#define _BYTE_ORDER     _BIG_ENDIAN
#else
#error "Please fix asm/byteorder.h"
#endif

#include <ieee80211_external.h>

#define ATH_XIOCTL_UNIFIED_UTF_CMD      0x1000
#define ATH_XIOCTL_UNIFIED_UTF_RSP      0x1001

#define CMD_TIMEOUT   10 

typedef struct {
    int sock;
    struct ifreq ifr;
    char ifname[IFNAMSIZ];
    void (*rx_cb)(void *buf);
    unsigned char initialized;
    unsigned char timeout;
    struct sigevent sev;
    timer_t timer;
} INIT_STRUCT;

static INIT_STRUCT initCfg;
static unsigned char responseBuf[2048+8];

int cmd_set_timer()
{
    struct itimerspec exp_time;
    int err;

    bzero(&exp_time, sizeof(exp_time));
    exp_time.it_value.tv_sec = CMD_TIMEOUT;
    err = timer_settime(initCfg.timer, 0, &exp_time, NULL);
    initCfg.timeout = 0;

    if (err < 0)
       return errno;

    return 0;
}

int cmd_stop_timer()
{
    struct itimerspec exp_time;
    int err;

    bzero(&exp_time, sizeof(exp_time));
    err = timer_settime(initCfg.timer, 0, &exp_time, NULL);

    if (err < 0)
       return errno;

    return 0;
}


static void timer_expire(union sigval sig)
{
    printf("Timer Expired..\n");
    initCfg.timeout = 1;
}

int cmd_init (char *ifname, void (*rx_cb)(void *buf))
{
    int ret = 0,s;

    if ( initCfg.initialized )
        return -1;

    memset(&initCfg.ifr, 0, sizeof(initCfg.ifr));
    strncpy(initCfg.ifr.ifr_name, ifname, IFNAMSIZ);

    initCfg.initialized = 1;

    initCfg.rx_cb = rx_cb;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        err(1, "socket(SOCK_DRAGM)");
        ret = -1;
    }

    initCfg.sock = s;

    // initCfg.sev.sigev_notify = SIGEV_THREAD;
    // initCfg.sev.sigev_notify_function = timer_expire;

    // timer_create(CLOCK_REALTIME,&initCfg.sev,&initCfg.timer);

    return ret;
}

int cmd_end()
{
    initCfg.initialized = 0;
	return 0;
}

void cmd_send (void *buf, int len, unsigned char responseNeeded )
{
    int error;
    unsigned int *responseCode;

    if (!initCfg.initialized)
       return;

    *(unsigned int *)buf = ATH_XIOCTL_UNIFIED_UTF_CMD;
    *((unsigned int *)buf + 1) = len;

    initCfg.ifr.ifr_data = (void *)buf;

    if (ioctl(initCfg.sock, SIOCIOCTLTX99, &initCfg.ifr) < 0) {
       err(1, "ioctl");
       return;
    }

    usleep(100000);

    if (responseNeeded) 
    {
        // cmd_set_timer();
        int time_counter=0;

        while (1)
        {
            memset(&responseBuf[0], 0, sizeof(responseBuf));
            *(unsigned int *)responseBuf = ATH_XIOCTL_UNIFIED_UTF_RSP;

            initCfg.ifr.ifr_data = (void *)responseBuf;

            error = ioctl(initCfg.sock, SIOCIOCTLTX99, &initCfg.ifr);
            
            time_counter++;
           if(time_counter==100000)
            {
                printf("%s[%d] Time out \n",__func__,__LINE__);
                memset(&responseBuf[0], 0, sizeof(responseBuf));
                initCfg.ifr.ifr_data = responseBuf;
                break;

            }
        responseCode = (unsigned int*)&initCfg.ifr.ifr_data[32];
      //printf("%s:: responseCode = 0x%x\n", __func__,*responseCode);
        if(*responseCode!=0) break;

        usleep(1000);

#if 0
            if ( initCfg.timeout )
            {
                memset(&responseBuf[0], 0, sizeof(responseBuf));
                initCfg.ifr.ifr_data = responseBuf;
                break;
            }

            if ( error < 0 )
            {
                if ( errno == EAGAIN )
                    continue;
                else
                {
                    printf("errno %d\n",errno);
                    memset(&responseBuf[0], 0, sizeof(responseBuf));
                    initCfg.ifr.ifr_data = responseBuf;
                    break;
                }
            }
#endif
        }

#if 0
        if(!initCfg.timeout)
        {
            cmd_stop_timer(); 
            initCfg.timeout = 0;
        }
#endif

        if ( initCfg.rx_cb != NULL )
            initCfg.rx_cb(initCfg.ifr.ifr_data);
    }
}

