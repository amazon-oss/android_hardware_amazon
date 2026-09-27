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

#include <unistd.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

using android::base::GetProperty;
using android::base::SetProperty;
using android::base::ReadFileToString;
using android::base::WriteStringToFile;

static bool is_valid_mac(const std::string& mac) {
    if (mac.size() < 12) return false;

    bool nonzero = false;
    for (size_t i = 0; i < 12; i++) {
        if (!isxdigit(mac[i])) return false;
        if (mac[i] != '0') nonzero = true;
    }

    return nonzero;
}

static std::string mac_from_serial(const std::string& salt) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : GetProperty("ro.serialno", "") + salt) {
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

static std::string read_idme_mac(const char* path, const std::string& salt) {
    std::string mac;

    if (!ReadFileToString(path, &mac) || !is_valid_mac(mac)) {
        LOG(WARNING) << "No valid MAC in " << path << ", deriving one from the serial number";
        mac = mac_from_serial(salt);
    }

    return mac;
}

static void load_bt_mac() {
    if (!set(kAndroidBtMacPath, parse_mac(read_idme_mac(kIdmeBtMacPath, "")))) {
        LOG(ERROR) << "Unable to write " << kAndroidBtMacPath;
        return;
    }

    SetProperty(kPropMacsAreReady, "1");
}

static void load_wifi_mac() {
    if (access(kBcmdhdModulePath, F_OK))
        return;

    // The driver applies it the next time Wi-Fi is brought up
    for (int i = 0; i < 50 && access(kBcmdhdMacParamPath, F_OK); i++)
        usleep(100 * 1000);

    if (!WriteStringToFile(parse_mac(read_idme_mac(kIdmeWifiMacPath, "wlan0")),
                           kBcmdhdMacParamPath))
        PLOG(ERROR) << "Unable to write " << kBcmdhdMacParamPath;
}

void load_amazon_idme() {
    load_bt_mac();
    load_wifi_mac();
}
