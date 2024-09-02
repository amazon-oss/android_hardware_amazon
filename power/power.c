/*
 * Copyright (C) 2014 The Android Open Source Project
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

#define LOG_TAG "Amazon Legacy PowerHAL"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#define DYNAMIC_BOOST_PATH "/sys/devices/platform/dynamic_boost/dynamic_boost"
#define NUM_POLICIES 2
#define MAX_BUF_SZ 64
#define LAUNCH_BOOST_TIME 5000 /* ms */
#define INTERACTION_BOOST_TIME 200 /* ms */

static const char *scaling_governor_paths[NUM_POLICIES] = {
    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor",
    "/sys/devices/system/cpu/cpu2/cpufreq/scaling_governor"
};

static const char *io_is_busy_paths[NUM_POLICIES] = {
    "/sys/devices/system/cpu/cpu0/cpufreq/%s/io_is_busy",
    "/sys/devices/system/cpu/cpu2/cpufreq/%s/io_is_busy"
};

static char governor[NUM_POLICIES][MAX_BUF_SZ];

typedef enum {
  PRIO_TWO_LITTLES,
  PRIO_TWO_LITTLES_MAX_FREQ,
  PRIO_MAX_CORES,
  PRIO_MAX_CORES_MAX_FREQ,
  PRIO_RESET,
  /* Define the max priority for priority limit */
  PRIO_DEFAULT
} dynamic_boost_mode_t;

static void sysfs_write(const char *path, const char *s) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        ALOGE("Unable to open %s: %s", path, strerror(errno));
        return;
    }

    if (write(fd, s, strlen(s)) < 0) {
        ALOGE("Failed to write to %s: %s", path, strerror(errno));
    }

    close(fd);
}

static int sysfs_read(const char *path, char *s, int num_bytes)
{
    int count;
    int ret = 0;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        ALOGE("Error opening %s: %s", path, strerror(errno));

        return -1;
    }

    if ((count = read(fd, s, num_bytes - 1)) < 0) {
        ALOGE("Error reading %s: %s", path, strerror(errno));

        ret = -1;
    } else {
        s[count] = '\0';
    }

    s[strcspn(s, "\n")] = 0;

    close(fd);

    return ret;
}

static int get_scaling_governor() {
    int rc = 0;

    for (int policy = 0; policy < NUM_POLICIES; policy++) {
        rc = sysfs_read(scaling_governor_paths[policy], governor[policy], MAX_BUF_SZ);
        if (rc < 0)
            break;
    }

    return rc;
}

static void power_set_dynamic_boost(dynamic_boost_mode_t mode, int durationMs)
{
    int fd;
    char buf[MAX_BUF_SZ];

    fd = open(DYNAMIC_BOOST_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("Unable to open %s: %s\n", DYNAMIC_BOOST_PATH, strerror(errno));
        return;
    }

    snprintf(buf, sizeof(buf), "%d %d\n", durationMs, mode);
    if (write(fd, buf, strlen(buf)) < 0) {
        ALOGE("Unable to write to %s: %s\n", DYNAMIC_BOOST_PATH, strerror(errno));
    }

    close(fd);
}

static void set_io_is_busy(int busy) {
    int rc;
    char path[MAX_BUF_SZ];

    rc = get_scaling_governor();
    if (rc < 0) {
        ALOGE("%s: Failed to read current governors.", __func__);
        return;
    }

    for (int policy = 0; policy < NUM_POLICIES; policy++) {
        snprintf(path, MAX_BUF_SZ, io_is_busy_paths[policy], governor[policy]);
        sysfs_write(path, busy ? "1" : "0");
        ALOGD("%s: Set %s to %d", __func__, path, busy);
    }
}

static void power_init(struct power_module *module __unused)
{
    return;
}

static void power_set_interactive(struct power_module *module __unused, int on) {
    ALOGD("%s: %s", __func__, on ? "ON" : "OFF");

    set_io_is_busy(on);
}

static void power_hint(struct power_module *module __unused, power_hint_t hint,
                       void *data __unused) {
    switch (hint) {
#ifndef HAS_TOUCH_BOOST
    case POWER_HINT_LAUNCH_BOOST:
        power_set_dynamic_boost(PRIO_MAX_CORES_MAX_FREQ, LAUNCH_BOOST_TIME);
        break;
#endif
    case POWER_HINT_INTERACTION:
        power_set_dynamic_boost(PRIO_TWO_LITTLES_MAX_FREQ, INTERACTION_BOOST_TIME);
        break;
    default:
        break;
    }
}

static void power_set_feature(struct power_module *module __unused, feature_t feature,
                              int state)
{
    return;
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_3,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Amazon Power HAL",
        .author = "The Android Open Source Project",
        .methods = &power_module_methods,
    },
    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
    .setFeature = power_set_feature,
};
