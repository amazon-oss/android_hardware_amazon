LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := metrics_logd_writer.c logd_writer.c
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE := libamazonlog
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
