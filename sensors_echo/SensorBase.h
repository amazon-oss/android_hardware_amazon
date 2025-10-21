/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_SENSOR_BASE_H
#define ANDROID_SENSOR_BASE_H
#define INSERT_FAKE_MAX 5
#define INSERT_DUR_MAX 8
#define INSERT_DUR_MIN 5

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <utils/Timers.h>

/*****************************************************************************/

struct sensors_event_t;

class SensorBase {
  protected:
    int data_fd;

    static int openInput(const char* inputName);

  public:
    SensorBase(int data_fd);

    virtual ~SensorBase();

    virtual int readEvents(sensors_event_t* data, int count) = 0;
    virtual bool hasPendingEvents() const;
    virtual int getFd() const;
    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled) = 0;
    virtual int batch(int handle, int flags, int64_t period_ns, int64_t timeout);
    virtual int isActivated(int handle);
};

/*****************************************************************************/

#endif  // ANDROID_SENSOR_BASE_H
