/*
 * Copyright (C) 2024 The LineageOS Project
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

const std::string kRushBoostPath     = "/proc/hps/rush_boost_enabled";
const std::string kFpsUpperBoundPath = "/d/ged/hal/fps_upper_bound"; 
const std::string kIoBusyPath        = "/sys/devices/system/cpu/cpufreq/interactive/io_is_busy";