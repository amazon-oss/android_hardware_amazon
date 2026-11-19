/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include "Idme.h"

using ::android::OK;
using ::android::sp;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::fireos::hardware::idme::V1_1::IIdme;
using ::fireos::hardware::idme::V1_1::implementation::Idme;

int main() {
    sp<IIdme> idme = new Idme();

    configureRpcThreadpool(1, true);

    if (idme->registerAsService() != OK) {
        LOG(ERROR) << "Failed to register IDME HAL";
        return EXIT_FAILURE;
    }

    joinRpcThreadpool();

    return EXIT_FAILURE;
}
