//
// Copyright (C) 2024 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "amazon_amonet"

#include "include/amazon_utils.h"
#include "include/amazon_amonet.h"

#include <android-base/logging.h>

#include <cerrno>
#include <cstring>
#include <vector>
#include <unistd.h>

const symlink_info_t symlinks[] = {
    {"boot", "boot_amonet", false},
    {"recovery", "recovery_amonet", false},
    {"boot_x", "boot", false},
    {"recovery_x", "recovery", false},
    {"lk", "lk_real", true},
    {"tee1", "tee1_real", true},
    {"tee2", "tee2_real", true},
};

static bool replace_symlink(const std::string& target, const std::string& link) {
    if (unlink(link.c_str()) && errno != ENOENT) {
        PLOG(ERROR) << "Unable to remove " << link;
        return false;
    }

    if (symlink(target.c_str(), link.c_str())) {
        PLOG(ERROR) << "Unable to link " << link << " to " << target;
        return false;
    }

    return true;
}

void load_amazon_amonet() {
    // The links are swapped in place, so only do it once per boot
    if (!access(symlinks[0].dst_name.c_str(), F_OK)) {
        LOG(INFO) << "Symlinks are already set up";
        return;
    }

    // Resolve everything first, so a missing partition leaves the layout untouched
    std::vector<std::string> targets;
    for (const auto& info : symlinks) {
        std::string target = resolve_symlink(info.src_name);
        if (target.empty()) {
            LOG(ERROR) << "Unable to resolve " << info.src_name << ", not an amonet layout";
            return;
        }
        targets.push_back(target);
    }

    for (size_t i = 0; i < targets.size(); i++) {
        const auto& info = symlinks[i];
        LOG(INFO) << "Creating symlink from " << info.src_name << " to " << info.dst_name;

        if (unlink(info.src_name.c_str()) && errno != ENOENT)
            PLOG(ERROR) << "Unable to remove " << info.src_name;

        replace_symlink(targets[i], info.dst_name);

        if (info.redirect_to_null)
            replace_symlink("/dev/null", info.src_name);
    }
}
