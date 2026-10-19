/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "amazon_init.h"

int main(int argc, char** argv) {
    SetupAmonetLinks();
    if (argc > 1 && !strcmp(argv[1], "--links")) return 0;

    SetupMacAddresses();
    return 0;
}
