/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_SENSORS_H
#define ANDROID_SENSORS_H

#include <errno.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include <hardware/hardware.h>
#include <hardware/sensors.h>

__BEGIN_DECLS

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device);

/*****************************************************************************/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define SENSOR_HANDLE(h) (SENSORS_HANDLE_BASE + h)

#define ID_LIGHT (0)
#define ID_PROXIMITY (1)

/*****************************************************************************/

/*
 * The SENSORS Module
 */
#define EVENT_TYPE_ALS_VALUE ABS_X
#define EVENT_TYPE_PS_VALUE REL_Z
#define EVENT_TYPE_ALS_STATUS ABS_WHEEL
#define EVENT_TYPE_PS_STATUS REL_Y

#define PROXIMITY_RANGE 5.0f

/*****************************************************************************/

__END_DECLS

#endif  // ANDROID_SENSORS_H
