//
// Copyright (C) 2024 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "amazon_idme"

#include "include/amazon_idme.h"
#include "include/amazon_utils.h"

#include <cctype>
#include <cstdint>
#include <cstdio>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

using android::base::GetProperty;
using android::base::SetProperty;
using android::base::ReadFileToString;

static bool is_valid_mac(const std::string& mac) {
    if (mac.size() < 12) return false;

    bool nonzero = false;
    for (size_t i = 0; i < 12; i++) {
        if (!isxdigit(mac[i])) return false;
        if (mac[i] != '0') nonzero = true;
    }

    return nonzero;
}

static std::string mac_from_serial() {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : GetProperty("ro.serialno", "")) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }

    // Locally administered, unicast
    char mac[13];
    snprintf(mac, sizeof(mac), "02%02x%02x%02x%02x%02x",
             static_cast<unsigned int>((hash >> 32) & 0xff),
             static_cast<unsigned int>((hash >> 24) & 0xff),
             static_cast<unsigned int>((hash >> 16) & 0xff),
             static_cast<unsigned int>((hash >> 8) & 0xff),
             static_cast<unsigned int>(hash & 0xff));

    return mac;
}

void load_amazon_idme() {
    std::string mac;

    if (!ReadFileToString(kIdmeBtMacPath, &mac) || !is_valid_mac(mac)) {
        LOG(WARNING) << "No valid Bluetooth MAC in " << kIdmeBtMacPath
                     << ", deriving one from the serial number";
        mac = mac_from_serial();
    }

    if (!set(kAndroidBtMacPath, parse_mac(mac))) {
        LOG(ERROR) << "Unable to write " << kAndroidBtMacPath;
        return;
    }

    SetProperty(kPropMacsAreReady, "1");
}
