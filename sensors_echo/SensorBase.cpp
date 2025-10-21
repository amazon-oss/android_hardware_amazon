/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "SensorBase"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <math.h>
#include <poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <utils/SystemClock.h>

#include <linux/input.h>

#include "SensorBase.h"

/*****************************************************************************/

SensorBase::SensorBase(int data_fd) : data_fd(data_fd) {}

SensorBase::~SensorBase() {
    if (data_fd >= 0) {
        close(data_fd);
        data_fd = -1;
    }
}

int SensorBase::getFd() const {
    return data_fd;
}

int SensorBase::setDelay(int32_t /* handle */, int64_t /* ns */) {
    return 0;
}

bool SensorBase::hasPendingEvents() const {
    return false;
}

struct input_dev {
    int fd;
    char name[80];
};

static int getInput(const char* inputName) {
    int fd = -1;
    unsigned i;
    static bool first = true;
    static struct input_dev dev[255];

    if (first) {
        int fd = -1;
        const char* dirname = "/dev/input";
        char devname[PATH_MAX];
        char* filename;
        DIR* dir;
        struct dirent* de;

        first = false;
        for (i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
            dev[i].fd = -1;
            dev[i].name[0] = '\0';
        }
        i = 0;

        dir = opendir(dirname);
        if (dir == NULL) return -1;
        strcpy(devname, dirname);
        filename = devname + strlen(devname);
        *filename++ = '/';
        while ((de = readdir(dir))) {
            if (de->d_name[0] == '.' &&
                (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0')))
                continue;
            strcpy(filename, de->d_name);
            fd = open(devname, O_RDONLY);
            if (fd >= 0) {
                char name[80];
                if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) >= 1) {
                    dev[i].fd = fd;
                    strncpy(dev[i].name, name, sizeof(dev[i].name));
                }
            }
            i++;
        }
        closedir(dir);
    }

    for (i = 0; i < sizeof(dev) / sizeof(dev[0]); i++) {
        if (!strncmp(inputName, dev[i].name, sizeof(dev[i].name))) {
            fd = dev[i].fd;
            break;
        }
    }
    if (fd < 0) ALOGE("couldn't find '%s' input device", inputName);

    return fd;
}

int SensorBase::openInput(const char* inputName) {
    int fd = -1;
    const char* dirname = "/dev/input";
    char devname[PATH_MAX];
    char* filename;
    DIR* dir;
    struct dirent* de;

    return getInput(inputName);

    dir = opendir(dirname);
    if (dir == NULL) return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while ((de = readdir(dir))) {
        if (de->d_name[0] == '.' &&
            (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd >= 0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
    if (fd < 0) ALOGE("couldn't find '%s' input device", inputName);

    return fd;
}

int SensorBase::batch(int /* handle */, int /* flags */, int64_t /* period_ns */,
                      int64_t /* timeout */) {
    return 0;
}

int SensorBase::enable(int32_t /* handle */, int /* enabled */) {
    return 0;
}

int SensorBase::isActivated(int /* handle */) {
    return 0;
}
