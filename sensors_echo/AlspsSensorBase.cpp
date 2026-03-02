/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "AlspsSensorBase"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <math.h>
#include <poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <utils/SystemClock.h>

#include "AlspsSensorBase.h"

/*****************************************************************************/
AlspsSensorBase::AlspsSensorBase(const char* inputFdPath, int32_t sensor, int32_t type)
    : SensorBase(getInputFdFromPath(inputFdPath)),
      mEnabled(0),
      mInputReader(sizeof(input_event)),
      mHasPendingEvent(false) {
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = sensor;
    mPendingEvent.type = type;
    memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
}

AlspsSensorBase::~AlspsSensorBase() {
    for (int& fd : mControlFds) {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }
    if (data_fd >= 0) {
        close(data_fd);
        data_fd = -1;
    }
}

int AlspsSensorBase::getInputFdFromPath(const char* path) {
    char buf[64], input_path[64];
    int input_dev, fd = open(path, O_RDONLY);
    ssize_t len;
    if (fd < 0) {
        ALOGE("Failed to open device number file: %s", path);
        return -1;
    }

    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (len <= 0) {
        ALOGE("Failed to read device number");
        return -1;
    }

    buf[len] = '\0';
    if (sscanf(buf, "%d", &input_dev) != 1) {
        ALOGE("Failed to parse device number from buffer: %s", buf);
        return -1;
    }

    snprintf(input_path, sizeof(input_path), "/dev/input/event%d", input_dev);
    fd = open(input_path, O_RDONLY);
    if (fd < 0) {
        ALOGE("Couldn't open input device: %s", input_path);
    }

    return fd;
}

int AlspsSensorBase::setDelay(int32_t /* handle */, int64_t ns) {
    char buf[64];
    int err, fd = mControlFds[DELAY_FD];

    if (fd < 0) {
        ALOGE("DELAY_FD not open");
        return -EINVAL;
    }

    snprintf(buf, sizeof(buf), "%lld", ns);
    err = write(fd, buf, sizeof(buf));
    return (err == sizeof(buf)) ? 0 : -EIO;
}

int AlspsSensorBase::batch(int /* handle */, int /* flags */, int64_t /* period_ns */, int64_t timeout) {
    char buf[2];
    int err, fd = mControlFds[BATCH_FD];

    if (fd < 0) {
        ALOGE("BATCH_FD not open");
        return -EINVAL;
    }

    buf[0] = (timeout == 0) ? '0' : '1';
    buf[1] = '\0';
    err = write(fd, buf, sizeof(buf));
    return (err == sizeof(buf)) ? 0 : -EIO;
}

int AlspsSensorBase::enable(int32_t, int en) {
    char buf[2];
    int err, fd = mControlFds[ACTIVE_FD];

    if (fd < 0) {
        ALOGE("ACTIVE_FD not open");
        return -EINVAL;
    }

    buf[0] = en ? '1' : '0';
    buf[1] = '\0';
    err = write(fd, buf, sizeof(buf));
    if (err != sizeof(buf)) return -EIO;

    mEnabled = !!en;
    setInitialState();
    return 0;
}

int AlspsSensorBase::isActivated(int /* handle */) {
    return mEnabled;
}

bool AlspsSensorBase::hasPendingEvents() const {
    return mHasPendingEvent;
}

int AlspsSensorBase::readEvents(sensors_event_t* data, int count) {
    int numEventReceived = 0;
    input_event const* event;
    ssize_t n;

    if (count < 1) return -EINVAL;

    if (mHasPendingEvent) {
        mHasPendingEvent = false;
        mPendingEvent.timestamp = android::elapsedRealtimeNano();
        *data = mPendingEvent;
        return mEnabled ? 1 : 0;
    }

    n = mInputReader.fill(data_fd);
    if (n < 0) return n;

    while (count && mInputReader.readEvent(&event)) {
        processEvent(data, count, numEventReceived, event);
        mInputReader.next();
    }

    return numEventReceived;
}
