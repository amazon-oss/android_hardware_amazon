//
// Copyright (C) 2024 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

#define LOG_TAG "amazon_idme"

#include "include/amazon_idme.h"
#include "include/amazon_utils.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

using android::base::SetProperty;
using android::base::ReadFileToString;

void load_amazon_idme() {
    std::string mac;

    if (!ReadFileToString(kIdmeBtMacPath, &mac)) {
        LOG(ERROR) << "Unable to read: " << kIdmeBtMacPath;
        return;
    }

    set(kAndroidBtMacPath, parse_mac(mac));
    SetProperty(kPropMacsAreReady, "1");
}
