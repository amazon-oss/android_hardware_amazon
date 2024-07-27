ifeq ($(TARGET_INIT_VENDOR_LIB),libinit_amazon)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := system/core/init
LOCAL_CFLAGS := -std=c99 -Wall -DANDROID_TARGET=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SRC_FILES := init_amazon.cpp init_amonet.cpp
ifneq ($(TARGET_LIBINIT_DEFINES_FILE),)
  LOCAL_SRC_FILES += ../../../$(TARGET_LIBINIT_DEFINES_FILE)
else
  LOCAL_SRC_FILES += init_$(TARGET_BOARD_PLATFORM).cpp
endif
LOCAL_MODULE := libinit_amazon
include $(BUILD_STATIC_LIBRARY)

endif
