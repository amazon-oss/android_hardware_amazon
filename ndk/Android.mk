LOCAL_PATH := prebuilts/ndk

include $(CLEAR_VARS)
LOCAL_MODULE := libstlport_shared
LOCAL_MODULE_STEM := libstlport
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_arm := current/sources/cxx-stl/stlport/libs/armeabi-v7a/libstlport_shared.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TARGET_ARCH := arm
LOCAL_MODULE_TAGS := optional
LOCAL_CHECK_ELF_FILES := false
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)
