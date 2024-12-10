LOCAL_PATH := prebuilts/ndk

include $(CLEAR_VARS)
LOCAL_MODULE := libstlport_shared
LOCAL_MODULE_STEM := libstlport
LOCAL_MULTILIB := both
LOCAL_SRC_FILES_arm := current/sources/cxx-stl/stlport/libs/armeabi-v7a/libstlport_shared.so
LOCAL_SRC_FILES_arm64 := current/sources/cxx-stl/stlport/libs/arm64-v8a/libstlport_shared.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TARGET_ARCH := arm arm64
LOCAL_MODULE_TAGS := optional
LOCAL_CHECK_ELF_FILES := false
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)
