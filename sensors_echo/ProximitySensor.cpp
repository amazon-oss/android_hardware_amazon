/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "ProximitySensor"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <log/log.h>
#include <math.h>
#include <poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <utils/SystemClock.h>

#include "ProximitySensor.h"

#define PS_CONTROL_PATH "/sys/class/misc/m_alsps_misc/"
#define PS_ACTIVE PS_CONTROL_PATH "psactive"
#define PS_BATCH PS_CONTROL_PATH "psbatch"
#define PS_DEVNUM PS_CONTROL_PATH "psdevnum"
#define PS_DELAY PS_CONTROL_PATH "psdelay"

/*****************************************************************************/
ProximitySensor::ProximitySensor()
    : AlspsSensorBase(PS_DEVNUM, ID_PROXIMITY, SENSOR_TYPE_PROXIMITY) {
    mControlFds[ACTIVE_FD] = open(PS_ACTIVE, O_WRONLY);
    if (mControlFds[ACTIVE_FD] < 0) {
        mControlFds[ACTIVE_FD] = -1;
        ALOGE("Failed to open ACTIVE control path.");
    }

    mControlFds[BATCH_FD] = open(PS_BATCH, O_WRONLY);
    if (mControlFds[BATCH_FD] < 0) {
        mControlFds[BATCH_FD] = -1;
        ALOGE("Failed to open BATCH control path.");
    }

    mControlFds[DELAY_FD] = open(PS_DELAY, O_WRONLY);
    if (mControlFds[DELAY_FD] < 0) {
        mControlFds[DELAY_FD] = -1;
        ALOGE("Failed to open DELAY control path.");
    }
}

void ProximitySensor::setInitialState() {
    struct input_absinfo absinfo;
    if ((data_fd > 0) && (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PS_VALUE), &absinfo))) {
        // make sure to report an event immediately
        mHasPendingEvent = true;
        mPendingEvent.distance = absinfo.value ? PROXIMITY_RANGE : 0.0f;
    }
}

void ProximitySensor::processEvent(sensors_event_t* data, int& count, int& numEventReceived,
                                   const input_event* event) {
    int type = event->type;

    if (type == EV_ABS) {
        if (event->code == EVENT_TYPE_PS_VALUE) {
            mPendingEvent.distance = event->value ? PROXIMITY_RANGE : 0.0f;
        }
    } else if (type == EV_SYN) {
        mPendingEvent.timestamp = android::elapsedRealtimeNano();
        if (mEnabled) {
            *data++ = mPendingEvent;
            count--;
            numEventReceived++;
        }
    } else {
        ALOGW("unknown event (type=%d, code=%d)", type, event->code);
    }
}
