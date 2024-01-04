LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.power@1.3-service.mt8163
LOCAL_INIT_RC := android.hardware.power@1.3-service.mt8163.rc
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
    service.cpp \
    Power.cpp

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libutils \
    libhidlbase \
    libhidltransport \
    android.hardware.power@1.0 \
    android.hardware.power@1.1 \
    android.hardware.power@1.2 \
    android.hardware.power@1.3

include $(BUILD_EXECUTABLE)
