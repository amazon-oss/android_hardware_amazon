LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_HEADER_LIBRARIES := libhardware_headers
LOCAL_SRC_FILES := lights.c
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE := lights.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
