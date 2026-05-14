LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libstagefright/MediaBufferGroup.cpp
LOCAL_SHARED_LIBRARIES := libstagefright_foundation libmedia
LOCAL_MODULE := libstagefright_shim
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libstlport/exception.cpp
LOCAL_MODULE := libstlport_shim
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libutils/StrongPointer.cpp
LOCAL_SHARED_LIBRARIES := libutils liblog
LOCAL_MODULE := libutils_shim
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libui/GraphicBuffer.cpp libui/GraphicBufferMapper.cpp
LOCAL_SHARED_LIBRARIES := libui
LOCAL_MODULE := libui_shim
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libgui/SensorManager.cpp
LOCAL_SHARED_LIBRARIES := libgui libutils
LOCAL_MODULE := libgui_shim
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libc/ioctl.cpp
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE := libc_shim
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS += -fno-stack-protector -U_FORTIFY_SOURCE -fno-sanitize=all
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := libbluetooth_mtk_pure/mtk.c
LOCAL_MODULE := libbluetooth_mtk_pure
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
