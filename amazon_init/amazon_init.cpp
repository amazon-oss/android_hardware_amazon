/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "amazon_init"

#include "include/amazon_amonet.h"
#include "include/amazon_idme.h"

#include <android-base/logging.h>

int main() {
    LOG(INFO) << "Running amazon_amonet";
    load_amazon_amonet();
    LOG(INFO) << "Running amazon_idme";
    load_amazon_idme();
    return 0;
}
