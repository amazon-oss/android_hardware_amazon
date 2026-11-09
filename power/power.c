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
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#define NUM_POLICIES 2
#define MAX_BUF_SZ 64
#define MAX_PATH_SZ 128

#define INTERACTION_BOOST_MS 500
#define LAUNCH_BOOST_MS 5000

static const char *policy_paths[NUM_POLICIES] = {
    "/sys/devices/system/cpu/cpu0/cpufreq",
    "/sys/devices/system/cpu/cpu2/cpufreq"
};

static char governor[NUM_POLICIES][MAX_BUF_SZ];

static pthread_mutex_t boost_lock = PTHREAD_MUTEX_INITIALIZER;
static timer_t boost_timer;
static bool boost_timer_created;
static bool boosted;
static uint64_t boost_end_ms;

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

static void get_scaling_governor() {
    char path[MAX_PATH_SZ];

    for (int policy = 0; policy < NUM_POLICIES; policy++) {
        governor[policy][0] = '\0';

        snprintf(path, MAX_PATH_SZ, "%s/scaling_governor", policy_paths[policy]);
        if (access(path, R_OK) == 0)
            sysfs_read(path, governor[policy], MAX_BUF_SZ);
    }
}

static void set_io_is_busy(int busy) {
    char path[MAX_PATH_SZ];

    get_scaling_governor();

    for (int policy = 0; policy < NUM_POLICIES; policy++) {
        if (governor[policy][0] == '\0')
            continue;

        snprintf(path, MAX_PATH_SZ, "%s/%s/io_is_busy", policy_paths[policy], governor[policy]);
        if (access(path, W_OK) < 0)
            snprintf(path, MAX_PATH_SZ, "/sys/devices/system/cpu/cpufreq/%s/io_is_busy",
                     governor[policy]);
        if (access(path, W_OK) < 0)
            continue;

        sysfs_write(path, busy ? "1" : "0");
        ALOGD("%s: Set %s to %d", __func__, path, busy);
    }
}

static void set_min_freq_boost(bool boost) {
    char path[MAX_PATH_SZ];
    char freq[MAX_BUF_SZ];

    for (int policy = 0; policy < NUM_POLICIES; policy++) {
        snprintf(path, MAX_PATH_SZ, "%s/scaling_min_freq", policy_paths[policy]);
        if (access(path, W_OK) < 0)
            continue;

        snprintf(path, MAX_PATH_SZ, "%s/%s", policy_paths[policy],
                 boost ? "cpuinfo_max_freq" : "cpuinfo_min_freq");
        if (sysfs_read(path, freq, MAX_BUF_SZ) < 0)
            continue;

        snprintf(path, MAX_PATH_SZ, "%s/scaling_min_freq", policy_paths[policy]);
        sysfs_write(path, freq);
    }
}

static uint64_t now_ms() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000;
}

static void end_boost() {
    struct itimerspec ts = {};

    pthread_mutex_lock(&boost_lock);

    if (boost_timer_created)
        timer_settime(boost_timer, 0, &ts, NULL);

    if (boosted) {
        set_min_freq_boost(false);
        boosted = false;
    }

    pthread_mutex_unlock(&boost_lock);
}

static void boost(int duration_ms) {
    uint64_t end = now_ms() + duration_ms;
    struct itimerspec ts = {
        .it_value = {
            .tv_sec = duration_ms / 1000,
            .tv_nsec = (duration_ms % 1000) * 1000000L,
        },
    };

    if (!boost_timer_created || duration_ms <= 0)
        return;

    pthread_mutex_lock(&boost_lock);

    if (!boosted || end > boost_end_ms) {
        boost_end_ms = end;
        timer_settime(boost_timer, 0, &ts, NULL);
    }

    if (!boosted) {
        set_min_freq_boost(true);
        boosted = true;
    }

    pthread_mutex_unlock(&boost_lock);
}

static void boost_timeout(union sigval sv __unused) {
    end_boost();
}

static void power_init(struct power_module *module __unused)
{
    struct sigevent sev = {
        .sigev_notify = SIGEV_THREAD,
        .sigev_notify_function = boost_timeout,
    };

    boost_timer_created = timer_create(CLOCK_MONOTONIC, &sev, &boost_timer) == 0;
    if (!boost_timer_created)
        ALOGE("Failed to create boost timer: %s", strerror(errno));
}

static void power_set_interactive(struct power_module *module __unused, int on) {
    ALOGD("%s: %s", __func__, on ? "ON" : "OFF");

    set_io_is_busy(on);

    if (!on)
        end_boost();
}

static void power_hint(struct power_module *module __unused, power_hint_t hint,
                       void *data) {
    switch (hint) {
        case POWER_HINT_INTERACTION:
            boost(INTERACTION_BOOST_MS);
            break;
        case POWER_HINT_CPU_BOOST:
            boost(data ? *(int *)data / 1000 : INTERACTION_BOOST_MS);
            break;
        case POWER_HINT_LAUNCH:
            if (data)
                boost(LAUNCH_BOOST_MS);
            else
                end_boost();
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
