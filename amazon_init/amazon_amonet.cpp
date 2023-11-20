//
// Copyright (C) 2023 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "amazon_amonet"

#include "include/amazon_utils.h"
#include "include/amazon_amonet.h"

#include <android-base/logging.h>

#include <fstream>
#include <iostream>
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

void load_amazon_amonet() {
    for (const auto &info : symlinks) {
        const char *resolved_symlink = resolve_symlink(info.src_name);
        LOG(INFO) << "Creating symlink from " << info.src_name
                  << " to " << info.dst_name;

        REMOVE_SYMLINK(info.src_name.c_str());
        CREATE_SYMLINK(resolved_symlink, info.dst_name.c_str());

        if (info.redirect_to_null) {
            CREATE_SYMLINK("/dev/null", info.src_name.c_str());
        }
    }
}
