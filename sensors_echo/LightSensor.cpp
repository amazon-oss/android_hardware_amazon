/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "LightSensor"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <math.h>
#include <poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <utils/SystemClock.h>

#include "LightSensor.h"

#define ALS_CONTROL_PATH "/sys/class/misc/m_alsps_misc/"
#define ALS_ACTIVE ALS_CONTROL_PATH "alsactive"
#define ALS_BATCH ALS_CONTROL_PATH "alsbatch"
#define ALS_DEVNUM ALS_CONTROL_PATH "alsdevnum"
#define ALS_DELAY ALS_CONTROL_PATH "alsdelay"

/*****************************************************************************/
LightSensor::LightSensor()
    : AlspsSensorBase(ALS_DEVNUM, ID_LIGHT, SENSOR_TYPE_LIGHT), mPreviousLight(-1.0f) {
    mControlFds[ACTIVE_FD] = open(ALS_ACTIVE, O_WRONLY);
    if (mControlFds[ACTIVE_FD] < 0) {
        mControlFds[ACTIVE_FD] = -1;
        ALOGE("Failed to open ACTIVE control path.");
    }

    mControlFds[BATCH_FD] = open(ALS_BATCH, O_WRONLY);
    if (mControlFds[BATCH_FD] < 0) {
        mControlFds[BATCH_FD] = -1;
        ALOGE("Failed to open BATCH control path.");
    }

    mControlFds[DELAY_FD] = open(ALS_DELAY, O_WRONLY);
    if (mControlFds[DELAY_FD] < 0) {
        mControlFds[DELAY_FD] = -1;
        ALOGE("Failed to open DELAY control path.");
    }
}

void LightSensor::setInitialState() {
    struct input_absinfo absinfo;
    if ((data_fd > 0) && (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ALS_VALUE), &absinfo))) {
        mPendingEvent.light = absinfo.value;
        if (mPendingEvent.light != mPreviousLight) {
            mHasPendingEvent = true;
            mPreviousLight = mPendingEvent.light;
        }
    }
}

void LightSensor::processEvent(sensors_event_t* data, int& count, int& numEventReceived,
                               const input_event* event) {
    int type = event->type;

    if (type == EV_ABS) {
        if (event->code == EVENT_TYPE_ALS_VALUE) {
            if (event->value >= 0) {
                mPendingEvent.light = event->value;
            } else {
                ALOGW("Dropping invalid event (type=%d, code=%d) with value %d", type, event->code,
                      event->value);
            }
        }
    } else if (type == EV_SYN) {
        mPendingEvent.timestamp = android::elapsedRealtimeNano();
        if (mEnabled && (mPendingEvent.light != mPreviousLight)) {
            *data++ = mPendingEvent;
            count--;
            numEventReceived++;
            mPreviousLight = mPendingEvent.light;
        }
    } else {
        ALOGW("unknown event (type=%d, code=%d)", type, event->code);
    }
}
