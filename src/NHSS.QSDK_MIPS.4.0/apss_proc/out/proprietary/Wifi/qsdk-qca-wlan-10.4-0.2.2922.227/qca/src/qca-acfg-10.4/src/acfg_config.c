#include <stdio.h>
#include <string.h>

int
acfg_get_ctrl_iface_path(char *filename, char *hapd_ctrl_iface_dir,
        char *wpa_supp_ctrl_iface_dir)
{
    FILE *fp;
    char buf[255], *pos;
    int len;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        pos = buf;
        if (strncmp(pos, "hostapd_ctrl_iface_dir=", 23)  == 0) {
            pos = strchr(buf, '=');
            pos++;
            len = strlen(pos);
            if (pos[len - 1] == '\n') {
                pos[len - 1] = '\0';
            }
            strcpy(hapd_ctrl_iface_dir, pos);
        } else if (strncmp(pos, "wpa_supp_ctrl_iface_dir=", 24) == 0) {
            pos = strchr(buf, '=');
            pos++;
            len = strlen(pos);
            if (pos[len - 1] == '\n') {
                pos[len - 1] = '\0';
            }
            strcpy(wpa_supp_ctrl_iface_dir, pos);
        }
    }
    fclose(fp);
    return 0;
}
