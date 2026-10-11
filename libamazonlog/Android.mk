#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wall -Werror -Wextra
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SRC_FILES := amazonlog.c
LOCAL_MODULE := libamazonlog
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
