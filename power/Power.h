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

#pragma once

#include <aidl/android/hardware/power/BnPower.h>

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace impl {
namespace mt8163 {

const std::string kRushBoostPath     = "/proc/hps/rush_boost_enabled";
const std::string kFpsUpperBoundPath = "/d/ged/hal/fps_upper_bound"; 
const std::string kIoBusyPath        = "/sys/devices/system/cpu/cpufreq/interactive/io_is_busy";
const std::string kDynamicBoostPath  = "/sys/devices/platform/dynamic_boost/dynamic_boost";

typedef enum DynamicBoostMode {
	PRIO_TWO_LITTLES,
	PRIO_TWO_LITTLES_MAX_FREQ,
	PRIO_ONE_BIG,
	PRIO_ONE_BIG_MAX_FREQ,
	PRIO_ONE_BIG_ONE_LITTLE,
	PRIO_ONE_BIG_ONE_LITTLE_MAX_FREQ,
	PRIO_TWO_BIGS,
	PRIO_TWO_BIGS_MAX_FREQ,
	PRIO_THREE_LITTLES,
	PRIO_THREE_LITTLES_MAX_FREQ,
	PRIO_FOUR_LITTLES,
	PRIO_FOUR_LITTLES_MAX_FREQ,
	PRIO_TWO_BIGS_TWO_LITTLES,
	PRIO_TWO_BIGS_TWO_LITTLES_MAX_FREQ,
	PRIO_FOUR_BIGS,
	PRIO_FOUR_BIGS_MAX_FREQ,
	PRIO_MAX_CORES,
	PRIO_MAX_CORES_MAX_FREQ,
	PRIO_RESET,
	PRIO_DEFAULT
} DynamicBoostMode;

class Power : public BnPower {
    ndk::ScopedAStatus setMode(Mode type, bool enabled) override;
    ndk::ScopedAStatus isModeSupported(Mode type, bool* _aidl_return) override;
    ndk::ScopedAStatus setBoost(Boost type, int32_t durationMs) override;
    ndk::ScopedAStatus isBoostSupported(Boost type, bool* _aidl_return) override;
};

}  // namespace mt8163
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
