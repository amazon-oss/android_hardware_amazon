ifeq ($(TARGET_INIT_VENDOR_LIB),libinit_amazon)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := system/core/init
LOCAL_CFLAGS := -std=c99 -Wall -DANDROID_TARGET=\"$(TARGET_BOARD_PLATFORM)\" -DLOCKED_BL=$(TARGET_HAS_LOCKED_BOOTLOADER)
LOCAL_SRC_FILES := init_amazon.c
ifneq ($(TARGET_LIBINIT_DEFINES_FILE),)
  LOCAL_SRC_FILES += ../../../$(TARGET_LIBINIT_DEFINES_FILE)
else
  LOCAL_SRC_FILES += init_$(TARGET_BOARD_PLATFORM).c
endif
ifeq ($(TARGET_HAS_LOCKED_BOOTLOADER), false)
  LOCAL_SRC_FILES += init_amonet.c
endif
LOCAL_MODULE := libinit_amazon
include $(BUILD_STATIC_LIBRARY)

endif
