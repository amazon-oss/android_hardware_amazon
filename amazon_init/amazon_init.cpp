/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "amazon_init"

#include "include/amazon_amonet.h"

#include <android-base/logging.h>

int main() {
    LOG(INFO) << "Running amazon_amonet";
    load_amazon_amonet();
    return 0;
}
