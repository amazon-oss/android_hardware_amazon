ifeq ($(TARGET_INIT_VENDOR_LIB),libinit_amazon)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := system/core/init
LOCAL_CFLAGS := -Wall -DANDROID_TARGET=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SRC_FILES := init_amazon.cpp init_amonet.cpp

ifneq ($(TARGET_LIBINIT_DEFINES_FILE),)
  LOCAL_SRC_FILES += ../../../$(TARGET_LIBINIT_DEFINES_FILE)
else
  ifneq ($(TARGET_BOARD_PLATFORM),)
    ifeq ($(wildcard $(LOCAL_PATH)/init_$(TARGET_BOARD_PLATFORM).cpp),$(LOCAL_PATH)/init_$(TARGET_BOARD_PLATFORM).cpp)
      LOCAL_SRC_FILES += init_$(TARGET_BOARD_PLATFORM).cpp
    endif
  endif
endif

LOCAL_MODULE := libinit_amazon
include $(BUILD_STATIC_LIBRARY)

endif
