/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_PROXIMITY_SENSOR_H
#define ANDROID_PROXIMITY_SENSOR_H

#include <errno.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "AlspsSensorBase.h"
#include "InputEventReader.h"
#include "SensorBase.h"
#include "nusensors.h"

/*****************************************************************************/

class ProximitySensor : public AlspsSensorBase {
  protected:
    virtual void setInitialState();
    virtual void processEvent(sensors_event_t* data, int& count, int& numEventReceived,
                              const input_event* event);

  public:
    ProximitySensor();
};
/*****************************************************************************/

#endif  // ANDROID_PROXIMITY_SENSOR_H
