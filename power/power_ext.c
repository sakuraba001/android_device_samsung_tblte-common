/*
 * Copyright (c) 2014 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define LOG_TAG "PowerHAL_TR_Ext"
#include <utils/Log.h>

#define NODE_MAX 64

#define GO_HISPEED_LOAD_PATH "/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load"
#define OFF_HIGHSPEED_LOAD 110

static int go_hispeed_load = 0;
static int off_hispeed_load = OFF_HIGHSPEED_LOAD;

#define TOUCHKEY_POWER "/sys/class/input/input1/enabled"
#define SPEN_POWER "/sys/class/input/input18/enabled"
#define TSP_POWER "/sys/class/input/input2/enabled"
#define LCD_POWER "/sys/class/power_supply/battery/lcd"

static int sysfs_read(char *path, char *s, int num_bytes)
{
    char buf[80];
    int count;
    int ret = 0;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);

        return -1;
    }

    if ((count = read(fd, s, num_bytes - 1)) < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error reading %s: %s\n", path, buf);

        ret = -1;
    } else {
        s[count] = '\0';
    }

    close(fd);

    return ret;
}

static void sysfs_write(char *path, char *s) {
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

void cm_power_set_interactive_ext(int on) {
    ALOGD("%s: %s input devices", __func__, on ? "enabling" : "disabling");
    sysfs_write(TSP_POWER, on ? "1" : "0");
    sysfs_write(TOUCHKEY_POWER, on ? "1" : "0");
    sysfs_write(SPEN_POWER, on ? "1" : "0");
    sysfs_write(LCD_POWER, on ? "1" : "0");

    char tmp_str[NODE_MAX];
    int tmp;

    if (sysfs_read(GO_HISPEED_LOAD_PATH, tmp_str, NODE_MAX - 1)) {
        return;
    }

    tmp = atoi(tmp_str);
    if (!go_hispeed_load || (go_hispeed_load != tmp && off_hispeed_load != tmp)) {
        go_hispeed_load = tmp;
    }

    snprintf(tmp_str, NODE_MAX, "%d", on ? go_hispeed_load : off_hispeed_load);
    sysfs_write(GO_HISPEED_LOAD_PATH, tmp_str);
}
