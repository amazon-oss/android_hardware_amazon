/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

const std::string kPathPrefix = "/dev/block/platform/soc/11230000.mmc/by-name/";

struct symlink_info_t {
    std::string src_name;
    std::string dst_name;
    bool redirect_to_null;

    symlink_info_t(const char* src, const char* dst, bool redirect)
        : src_name(kPathPrefix + src),
          dst_name(kPathPrefix + dst),
          redirect_to_null(redirect) {}
};

void load_amazon_amonet();
