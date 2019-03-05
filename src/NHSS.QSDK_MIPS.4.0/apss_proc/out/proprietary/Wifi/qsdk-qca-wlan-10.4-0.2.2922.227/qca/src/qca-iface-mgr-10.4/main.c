/*
 * Copyright (c) 2016 Qualcomm Atheros, Inc..
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <dirent.h>
#include <sys/un.h>

#include <pthread.h>
#include "includes.h"
#include "eloop.h"
#include "common.h"
#include "wpa_ctrl.h"
#include "iface_mgr_api.h"
#include <fcntl.h>

ifacemgr_hdl_t *ifacemgr_handle;
#define MAX_PID_LEN 6
int store_pid(const char *pid_file) {
    if (pid_file) {
	int f = open(pid_file, O_WRONLY | O_CREAT);
	int pid = getpid();
	char buf[MAX_PID_LEN] = {0};
	snprintf(buf, MAX_PID_LEN, "%d", pid);
	if (f) {
	    if (write(f, buf, strlen(buf)) != strlen(buf)) {
		ifacemgr_printf("Failed to write pid file");
		return -1;
	    }
	    close(f);
	    return 0;
	}
    }
    return -1;
}



int main(int argc, char *argv[])
{
    const char *conf_file = "/var/run/iface_mgr.conf";
    const char *global_wpa_s_ctrl_intf = "/var/run/wpa_supplicantglobal";
    const char *pid_file = "/var/run/iface_mgr.pid";
    void *son_conf = "/var/run/son.conf";

    int ret = 0;

    pthread_t son_main_func;
    pthread_create(&son_main_func, NULL, son_main_function, son_conf);
    pthread_join(son_main_func, NULL);

    ifacemgr_handle = ifacemgr_load_conf_file(conf_file);
    if (ifacemgr_handle == NULL) {
	ifacemgr_printf("Failed to load conf file");
        goto out;
    }
    if (eloop_init()) {
	ifacemgr_printf("Failed to initialize event loop");
	goto out;
    }

    ret = ifacemgr_conn_to_global_wpa_s((const char*)global_wpa_s_ctrl_intf, ifacemgr_handle);
    if (!ret) {
	ifacemgr_printf("Failed to establish connection with global wpa supplicant");
	goto out;
    }

    ret = ifacemgr_update_ifacemgr_handle_config(ifacemgr_handle);
    if (!ret) {
	ifacemgr_printf("Failed to update ifacemge handler config");
	goto out;
    }

    if(store_pid(pid_file)) {
	ifacemgr_printf("Unable to store PID in file");
	goto out;
    }

    ifacemgr_display_updated_config(ifacemgr_handle);

    eloop_run();

    return 1;

out:
    if (ifacemgr_handle) {
        ifacemgr_status_inform_to_driver(ifacemgr_handle, 0);
        ifacemgr_free_ifacemgr_handle(ifacemgr_handle);
    }
    return 0;

}

