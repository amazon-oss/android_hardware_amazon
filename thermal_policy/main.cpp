/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <android-base/logging.h>
#include <android-base/properties.h>

#include "ThermalPolicy.h"

using android::base::GetBoolProperty;

constexpr char kPolicy[] = "/vendor/etc/.tp/thermal.policy.conf";
constexpr char kWpcPolicy[] = "/vendor/etc/.tp/thermal.wpc.policy.conf";

int main() {
    const char* path = kPolicy;
    if (GetBoolProperty("ro.boot.wpc.support", false) && access(kWpcPolicy, F_OK) == 0)
        path = kWpcPolicy;

    ThermalPolicy policy;
    if (!policy.Load(path)) {
        LOG(INFO) << "No thermal policy at " << path;
        return 0;
    }

    policy.Apply();

    return 0;
}
