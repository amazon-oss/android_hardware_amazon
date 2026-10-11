/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "amazon_init"

#include "amazon_init.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <string>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

using android::base::GetProperty;
using android::base::ReadFileToString;
using android::base::SetProperty;
using android::base::WriteStringToFile;

namespace {

constexpr char kIdmeBtMac[] = "/proc/idme/bt_mac_addr";
constexpr char kIdmeWifiMac[] = "/proc/idme/mac_addr";
constexpr char kBtMacFile[] = "/data/vendor/bluedroid/macbt";
constexpr char kBcmdhdModule[] = "/vendor/lib/modules/amzn-bcmdhd.ko";
constexpr char kBcmdhdMacParam[] = "/sys/module/amzn_bcmdhd/parameters/iface_wlan0_mac";

bool IsValidMac(const std::string& mac) {
    if (mac.size() < 12) return false;

    bool nonzero = false;
    for (size_t i = 0; i < 12; i++) {
        if (!isxdigit(mac[i])) return false;
        if (mac[i] != '0') nonzero = true;
    }

    return nonzero;
}

std::string MacFromSerial(const std::string& salt) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : GetProperty("ro.serialno", "") + salt) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }

    char mac[13];
    snprintf(mac, sizeof(mac), "02%010llx",
             static_cast<unsigned long long>(hash & 0xffffffffffULL));
    return mac;
}

std::string ReadMac(const char* path, const std::string& salt) {
    std::string mac;
    if (ReadFileToString(path, &mac) && IsValidMac(mac)) return mac.substr(0, 12);

    LOG(WARNING) << "No valid MAC in " << path << ", deriving one from the serial number";
    return MacFromSerial(salt);
}

std::string FormatMac(const std::string& mac) {
    std::string formatted;
    for (size_t i = 0; i < 12; i += 2) {
        if (i) formatted += ':';
        formatted += mac.substr(i, 2);
    }
    return formatted;
}

void SetupBtMac() {
    if (!WriteStringToFile(FormatMac(ReadMac(kIdmeBtMac, "")) + "\n", kBtMacFile)) {
        PLOG(ERROR) << "Failed to write " << kBtMacFile;
        return;
    }

    SetProperty("sys.conn_macs.ready", "1");
}

void SetupWifiMac() {
    if (access(kBcmdhdModule, F_OK)) return;

    for (int i = 0; i < 50 && access(kBcmdhdMacParam, F_OK); i++) usleep(100 * 1000);

    if (!WriteStringToFile(FormatMac(ReadMac(kIdmeWifiMac, "wlan0")), kBcmdhdMacParam))
        PLOG(ERROR) << "Failed to write " << kBcmdhdMacParam;
}

}  // namespace

void SetupMacAddresses() {
    SetupBtMac();
    SetupWifiMac();
}
