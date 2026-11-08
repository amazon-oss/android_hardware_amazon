#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

ifeq ($(TARGET_INIT_VENDOR_LIB),libinit_amazon)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := system/core/init
LOCAL_CFLAGS := -Wall -Werror -Wextra
LOCAL_SRC_FILES := init_amazon.cpp
LOCAL_MODULE := libinit_amazon
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

endif
