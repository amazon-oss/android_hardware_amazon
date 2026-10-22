//
// SPDX-FileCopyrightText: 2016 The Android Open Source Project
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "fireosaspservice"

#include <signal.h>
#include <string>
#include <vector>

#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <log/log.h>

using namespace android::hardware;
using android::OK;

int main() {
    signal(SIGPIPE, SIG_IGN);

    ::android::ProcessState::initWithDriver("/dev/vndbinder");
    ::android::ProcessState::self()->startThreadPool();

    configureRpcThreadpool(4, true /* callerWillJoin */);

    const std::vector<std::string> interfaces = {
        "fireos.hardware.audiosignalprocessor@1.1::IHalAudioSignalProcessor",
        "fireos.hardware.audiosignalprocessor@1.0::IHalAudioSignalProcessor",
    };

    bool registered = false;
    for (const auto& interface : interfaces) {
        if (registerPassthroughServiceImplementation(interface) == OK) {
            registered = true;
            break;
        }
    }

    if (!registered) {
        ALOGE("Could not register IHalAudioSignalProcessor");
        return 1;
    }

    joinRpcThreadpool();
    return 1;
}
