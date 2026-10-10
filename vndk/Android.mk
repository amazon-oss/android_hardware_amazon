LOCAL_PATH := prebuilts/vndk

ifeq ($(TARGET_ARCH_VARIANT),armv7-a-neon)
VNDK_ARM_DIR := arm/arch-arm-armv7-a-neon
else
VNDK_ARM_DIR := arm64/arch-arm-armv8-a
endif

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.audio.common@4.0-util-v28
LOCAL_MULTILIB := both
LOCAL_SRC_FILES_arm := v28/$(VNDK_ARM_DIR)/shared/vndk-core/android.hardware.audio.common@4.0-util.so
LOCAL_SRC_FILES_arm64 := v28/arm64/arch-arm-armv8-a/shared/vndk-core/android.hardware.audio.common@4.0-util.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TARGET_ARCH := arm arm64
LOCAL_MODULE_TAGS := optional
LOCAL_CHECK_ELF_FILES := false
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libcompiler_rt-v29
LOCAL_MULTILIB := both
LOCAL_SRC_FILES_arm := v29/$(VNDK_ARM_DIR)/shared/vndk-sp/libcompiler_rt.so
LOCAL_SRC_FILES_arm64 := v29/arm64/arch-arm64-armv8-a/shared/vndk-sp/libcompiler_rt.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TARGET_ARCH := arm arm64
LOCAL_MODULE_TAGS := optional
LOCAL_CHECK_ELF_FILES := false
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE := libprotobuf-cpp-lite-v29
LOCAL_MULTILIB := both
LOCAL_SRC_FILES_arm := v29/$(VNDK_ARM_DIR)/shared/vndk-core/libprotobuf-cpp-lite.so
LOCAL_SRC_FILES_arm64 := v29/arm64/arch-arm64-armv8-a/shared/vndk-core/libprotobuf-cpp-lite.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TARGET_ARCH := arm arm64
LOCAL_MODULE_TAGS := optional
LOCAL_CHECK_ELF_FILES := false
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libui-v28
LOCAL_MULTILIB := both
LOCAL_SRC_FILES_arm := v28/$(VNDK_ARM_DIR)/shared/vndk-core/libui.so
LOCAL_SRC_FILES_arm64 := v28/arm64/arch-arm64-armv8-a/shared/vndk-core/libui.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TARGET_ARCH := arm arm64
LOCAL_MODULE_TAGS := optional
LOCAL_CHECK_ELF_FILES := false
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)
