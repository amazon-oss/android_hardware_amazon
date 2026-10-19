/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "amazon_init"

#include "amazon_init.h"

#include <errno.h>
#include <unistd.h>

#include <string>
#include <vector>

#include <android-base/file.h>
#include <android-base/logging.h>

using android::base::Realpath;

namespace {

const std::string kByName = "/dev/block/platform/soc/11230000.mmc/by-name/";

struct AmonetLink {
    const char* partition;
    const char* alias;
    bool hide;
    bool optional;
};

constexpr AmonetLink kLinks[] = {
        {"boot", "boot_amonet", false, false}, {"recovery", "recovery_amonet", false, false},
        {"boot_x", "boot", false, false},      {"recovery_x", "recovery", false, false},
        {"lk", "lk_real", true, false},        {"lk2", "lk2_real", true, true},
        {"tee1", "tee1_real", true, false},    {"tee2", "tee2_real", true, false},
        {"boot0hdr0", "boot0hdr0_real", true, true}, {"boot0hdr1", "boot0hdr1_real", true, true},
        {"boot0img0", "boot0img0_real", true, true}, {"boot0img1", "boot0img1_real", true, true},
};

void Remove(const std::string& path) {
    if (unlink(path.c_str()) && errno != ENOENT) PLOG(ERROR) << "Failed to remove " << path;
}

void Link(const std::string& target, const std::string& path) {
    Remove(path);
    if (symlink(target.c_str(), path.c_str()))
        PLOG(ERROR) << "Failed to link " << path << " to " << target;
}

}  // namespace

void SetupAmonetLinks() {
    if (!access((kByName + kLinks[0].alias).c_str(), F_OK)) return;

    std::vector<std::string> targets;
    for (const auto& link : kLinks) {
        std::string target;
        if (!Realpath(kByName + link.partition, &target) && !link.optional) {
            LOG(INFO) << "No " << link.partition << " partition, not an amonet layout";
            return;
        }
        targets.push_back(target);
    }

    for (size_t i = 0; i < targets.size(); i++) {
        if (targets[i].empty()) continue;

        const std::string partition = kByName + kLinks[i].partition;

        Remove(partition);
        Link(targets[i], kByName + kLinks[i].alias);
        if (kLinks[i].hide) Link("/dev/null", partition);
    }
}
