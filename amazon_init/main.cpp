/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "amazon_init.h"

int main() {
    SetupAmonetLinks();
    SetupMacAddresses();
    return 0;
}
