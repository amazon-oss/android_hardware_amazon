//
// SPDX-FileCopyrightText: 2016 The Android Open Source Project
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "fireosaudiohalservice"

#include <signal.h>
#include <string>
#include <vector>

#include <binder/ProcessState.h>
#include <cutils/properties.h>
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

int main(int /* argc */, char* /* argv */[]) {
    signal(SIGPIPE, SIG_IGN);

    ::android::ProcessState::initWithDriver("/dev/vndbinder");
    ::android::ProcessState::self()->startThreadPool();

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
            "fireos.hardware.audio@4.0::IDevicesFactory",
            "fireos.hardware.audio@2.0::IDevicesFactory",
        },
        {
            "Audio Effect API",
            "android.hardware.audio.effect@4.0::IEffectsFactory",
            "android.hardware.audio.effect@2.0::IEffectsFactory",
        },
    };

    const std::vector<InterfacesList> optionalInterfaces = {
        {
            "Audiosignalprocessor API",
            "fireos.hardware.audiosignalprocessor@1.1::IHalAudioSignalProcessor",
            "fireos.hardware.audiosignalprocessor@1.0::IHalAudioSignalProcessor",
        },
        {
            "Bluetooth Audio API",
            "android.hardware.bluetooth.audio@2.0::IBluetoothAudioProvidersFactory",
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

    joinRpcThreadpool();
}
