/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <errno.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "InputEventReader.h"
#include "SensorBase.h"
#include "nusensors.h"

/*****************************************************************************/

struct input_event;

class AlspsSensorBase : public SensorBase {
  protected:
    enum {
        ACTIVE_FD,
        BATCH_FD,
        DELAY_FD,
        NUM_FDS,
    };
    int mEnabled;
    int mControlFds[NUM_FDS];
    InputEventCircularReader mInputReader;
    sensors_event_t mPendingEvent;
    bool mHasPendingEvent;

    virtual void setInitialState() = 0;
    virtual void processEvent(sensors_event_t* data, int& count, int& numEventReceived,
                              const input_event* event) = 0;

  private:
    static int getInputFdFromPath(const char* path);

  public:
    AlspsSensorBase(const char* inputFdPath, int32_t sensor, int32_t type);
    virtual ~AlspsSensorBase();
    virtual int readEvents(sensors_event_t* data, int count);
    virtual bool hasPendingEvents() const;
    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled);
    virtual int batch(int handle, int flags, int64_t period_ns, int64_t timeout);
    virtual int isActivated(int handle);
};
/*****************************************************************************/
