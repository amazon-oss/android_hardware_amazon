/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

constexpr const char* kAndroidBtMacPath = "/data/vendor/bluedroid/macbt";
constexpr const char* kIdmeBtMacPath = "/proc/idme/bt_mac_addr";
constexpr const char* kPropMacsAreReady = "sys.conn_macs.ready";

constexpr const char* kIdmeWifiMacPath = "/proc/idme/mac_addr";
constexpr const char* kBcmdhdModulePath = "/vendor/lib/modules/amzn-bcmdhd.ko";
constexpr const char* kBcmdhdMacParamPath = "/sys/module/amzn_bcmdhd/parameters/iface_wlan0_mac";

void load_amazon_idme();
