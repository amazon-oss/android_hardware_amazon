#
# SPDX-FileCopyrightText: 2025 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# HAL module implementation, not prelinked, and stored in
# hw/<SENSORS_HARDWARE_MODULE_ID>.echo.so
LOCAL_MODULE := sensors.echo
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    AlspsSensorBase.cpp \
    InputEventReader.cpp \
    LightSensor.cpp \
    nusensors.cpp \
    ProximitySensor.cpp \
    SensorBase.cpp \
    sensors.c

LOCAL_SHARED_LIBRARIES := liblog libcutils libutils

ifeq ($(BOARD_LIGHT_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DLIGHT_SENSOR_SUPPORT
endif
ifeq ($(BOARD_PROXIMITY_SENSOR_SUPPORT), true)
LOCAL_CFLAGS += -DPROXIMITY_SENSOR_SUPPORT
endif

include $(BUILD_SHARED_LIBRARY)
