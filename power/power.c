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

#include <cutils/properties.h>

#define DYNAMIC_BOOST_PATH "/sys/devices/platform/dynamic_boost/dynamic_boost"
#define MAX_BUF_SZ 15
#define LAUNCH_BOOST_TIME 5000 /* ms */
#define INTERACTION_BOOST_TIME 200 /* ms */
#define POWER_PROFILE_PROP "sys.perf.profile"

typedef enum {
  PRIO_TWO_LITTLES,
  PRIO_TWO_LITTLES_MAX_FREQ,
  PRIO_MAX_CORES,
  PRIO_MAX_CORES_MAX_FREQ,
  PRIO_RESET,
  /* Define the max priority for priority limit */
  PRIO_DEFAULT
} dynamic_boost_mode_t;

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

static void power_init(struct power_module *module __unused)
{
    return;
}

static void power_set_interactive(struct power_module *module __unused, int on) {
    ALOGD("%s: %s", __func__, (on ? "ON" : "OFF"));
    if (on) {
        char profile[PROPERTY_VALUE_MAX];
        property_get(POWER_PROFILE_PROP, profile, "0");

        if (atoi(profile) == 2) {
            ALOGD("%s: Restore indefinite dynamic boost for performance mode", __func__);
            power_set_dynamic_boost(PRIO_MAX_CORES_MAX_FREQ, -1);
        }
    }
}

static void power_hint(struct power_module *module __unused, power_hint_t hint,
                       void *data __unused) {
    char profile[PROPERTY_VALUE_MAX];

    property_get(POWER_PROFILE_PROP, profile, "0");
    if (atoi(profile) == 2)
        return;

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
