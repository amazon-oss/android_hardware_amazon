/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <hardware/sensors.h>

#include "nusensors.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

static const struct sensor_t sSensorList[] = {
#ifdef PROXIMITY_SENSOR_SUPPORT
        {.name = "Proximity Sensor",
         .vendor = "amazon-oss",
         .version = 1,
         .handle = SENSOR_HANDLE(ID_PROXIMITY),
         .type = SENSOR_TYPE_PROXIMITY,
         .maxRange = PROXIMITY_RANGE,
         .resolution = 1.0f,
         .power = 0.13f,
         .minDelay = 0,
         .fifoReservedEventCount = 0,
         .fifoMaxEventCount = 0,
         .stringType = SENSOR_STRING_TYPE_PROXIMITY,
         .requiredPermission = 0,
         .maxDelay = 0,
         .flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP,
         .reserved = {}},
#endif
#ifdef LIGHT_SENSOR_SUPPORT
        {.name = "Light Sensor",
         .vendor = "amazon-oss",
         .version = 1,
         .handle = SENSOR_HANDLE(ID_LIGHT),
         .type = SENSOR_TYPE_LIGHT,
         .maxRange = 10240.0f,
         .resolution = 1.0f,
         .power = 0.13f,
         .minDelay = 0,
         .fifoReservedEventCount = 0,
         .fifoMaxEventCount = 0,
         .stringType = SENSOR_STRING_TYPE_LIGHT,
         .requiredPermission = 0,
         .maxDelay = 0,
         .flags = SENSOR_FLAG_ON_CHANGE_MODE,
         .reserved = {}},
#endif
};

static int open_sensors(const struct hw_module_t* module, const char* name,
                        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
                                     struct sensor_t const** list) {
    (void)module;
    *list = sSensorList;
    return ARRAY_SIZE(sSensorList);
}

static struct hw_module_methods_t sensors_module_methods = {.open = open_sensors};

struct sensors_module_t HAL_MODULE_INFO_SYM = {.common =
                                                       {
                                                               .tag = HARDWARE_MODULE_TAG,
                                                               .version_major = 1,
                                                               .version_minor = 0,
                                                               .id = SENSORS_HARDWARE_MODULE_ID,
                                                               .name = "Amazon Echo Sensors Module",
                                                               .author = "amazon-oss",
                                                               .methods = &sensors_module_methods,
                                                       },
                                               .get_sensors_list = sensors__get_sensors_list};

/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* name,
                        struct hw_device_t** device) {
    (void)name;
    return init_nusensors(module, device);
}
