//
// SPDX-FileCopyrightText: 2016 The Android Open Source Project
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "fireosaudiohalservice"

#include <signal.h>
#include <string>
#include <utility>
#include <vector>

#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <hwbinder/ProcessState.h>

using namespace android::hardware;
using android::OK;

using InterfacesList = std::vector<std::string>;

template <class Iter>
static bool registerPassthroughServiceImplementations(Iter first, Iter last) {
    for (; first != last; ++first) {
        if (registerPassthroughServiceImplementation(*first) == OK) {
            return true;
        }
    }
    return false;
}

static bool registerExternalServiceImplementation(const std::string& libName,
                                                  const std::string& funcName) {
    dlerror();
    auto libPath = libName + ".so";
    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        const char* error = dlerror();
        ALOGE("Failed to dlopen %s: %s", libPath.c_str(), error != nullptr ? error : "unknown error");
        return false;
    }

    binder_status_t (*factoryFunction)();
    *(void**)(&factoryFunction) = dlsym(handle, funcName.c_str());
    if (!factoryFunction) {
        const char* error = dlerror();
        ALOGE("Factory function %s not found in %s: %s", funcName.c_str(), libPath.c_str(),
              error != nullptr ? error : "unknown error");
        dlclose(handle);
        return false;
    }

    return (*factoryFunction)() == STATUS_OK;
}

int main(int /* argc */, char* /* argv */[]) {
    signal(SIGPIPE, SIG_IGN);

    if (::android::ProcessState::isVndservicemanagerEnabled()) {
        ::android::ProcessState::initWithDriver("/dev/vndbinder");
        ::android::ProcessState::self()->startThreadPool();
    }

    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();

    const int32_t defaultValue = -1;
    int32_t value =
            property_get_int32("persist.vendor.audio.service.hwbinder.size_kbyte", defaultValue);
    if (value != defaultValue) {
        ALOGD("Configuring hwbinder with mmap size %d KBytes", value);
        ProcessState::initWithMmapSize(static_cast<size_t>(value) * 1024);
    }
    configureRpcThreadpool(16, true /* callerWillJoin */);

    // clang-format off
    const std::vector<InterfacesList> mandatoryInterfaces = {
        {
            "Audio Core API",
            "fireos.hardware.audio@6.0::IDevicesFactory",
        },
        {
            "Audio Effect API",
            "android.hardware.audio.effect@6.0::IEffectsFactory",
        },
    };

    const std::vector<InterfacesList> optionalInterfaces = {
        {
            "Audiosignalprocessor API",
            "fireos.hardware.audiosignalprocessor@1.1::IHalAudioSignalProcessor",
            "fireos.hardware.audiosignalprocessor@1.0::IHalAudioSignalProcessor",
        },
    };

    const std::vector<std::pair<std::string, std::string>> optionalInterfaceSharedLibs = {
        {
            "android.hardware.bluetooth.audio-impl",
            "createIBluetoothAudioProviderFactory",
        },
    };
    // clang-format on

    for (const auto& listIter : mandatoryInterfaces) {
        auto iter = listIter.begin();
        const std::string& interfaceFamilyName = *iter++;
        LOG_ALWAYS_FATAL_IF(!registerPassthroughServiceImplementations(iter, listIter.end()),
                            "Could not register %s", interfaceFamilyName.c_str());
    }

    for (const auto& listIter : optionalInterfaces) {
        auto iter = listIter.begin();
        const std::string& interfaceFamilyName = *iter++;
        ALOGW_IF(!registerPassthroughServiceImplementations(iter, listIter.end()),
                 "Could not register %s", interfaceFamilyName.c_str());
    }

    for (const auto& interfacePair : optionalInterfaceSharedLibs) {
        const std::string& libraryName = interfacePair.first;
        const std::string& interfaceLoaderFuncName = interfacePair.second;
        if (registerExternalServiceImplementation(libraryName, interfaceLoaderFuncName)) {
            ALOGI("%s() from %s success", interfaceLoaderFuncName.c_str(), libraryName.c_str());
        } else {
            ALOGW("%s() from %s failed", interfaceLoaderFuncName.c_str(), libraryName.c_str());
        }
    }

    joinRpcThreadpool();
}
