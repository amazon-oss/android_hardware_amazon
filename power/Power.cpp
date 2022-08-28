/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Power.h"

#include <fstream>

#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace impl {
namespace mt8163 {

template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value << std::endl;
}

static void setDynamicBoost(const DynamicBoostMode mode, const int32_t durationMs) {
    set(kDynamicBoostPath, std::to_string(durationMs) + " " + std::to_string(static_cast<int32_t>(mode)));
}

ndk::ScopedAStatus Power::setMode(Mode type, bool enabled) {
    ndk::ScopedAStatus status;

    LOG(VERBOSE) << "Power setMode: " << static_cast<int32_t>(type) << " to: " << enabled;
    switch (type) {
        case Mode::LOW_POWER:
            if (enabled) {
                set(kRushBoostPath,  "0");
                set(kFpsUpperBoundPath, "30");
            } else {
                set(kRushBoostPath, "1");
                set(kFpsUpperBoundPath, "60");
            }
            status = ndk::ScopedAStatus::ok();
            break;
        case Mode::LAUNCH:
            if (enabled)
                setDynamicBoost(DynamicBoostMode::PRIO_MAX_CORES_MAX_FREQ, 5000);
            else
                setDynamicBoost(DynamicBoostMode::PRIO_RESET, 0);
            status = ndk::ScopedAStatus::ok();
            break;
        default:
            status = ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
            break;
    }
    return status;
}

ndk::ScopedAStatus Power::isModeSupported(Mode type, bool* _aidl_return) {
    LOG(INFO) << "Power isModeSupported: " << static_cast<int32_t>(type);
    switch (type) {
        case Mode::LOW_POWER:
            *_aidl_return = true;
            break;
        case Mode::LAUNCH:
            *_aidl_return = true;
            break;
        default:
            break;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::setBoost(Boost type, int32_t durationMs) {
    ndk::ScopedAStatus status;

    LOG(VERBOSE) << "Power setBoost: " << static_cast<int32_t>(type)
                 << ", duration: " << durationMs;

    switch (type) {
        case Boost::INTERACTION:
            if (durationMs > 0)
                setDynamicBoost(DynamicBoostMode::PRIO_TWO_LITTLES_MAX_FREQ, durationMs);
            status = ndk::ScopedAStatus::ok();
            break;
        default:
            status = ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
            break;
    }
    return status;
}

ndk::ScopedAStatus Power::isBoostSupported(Boost type, bool* _aidl_return) {
    LOG(INFO) << "Power isBoostSupported: " << static_cast<int32_t>(type);
    switch (type) {
        case Boost::INTERACTION:
            *_aidl_return = true;
            break;
        default:
            break;
    }
    return ndk::ScopedAStatus::ok();
}

}  // namespace mt8163
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
