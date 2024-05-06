/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define REMOVE_SYMLINK(path) \
    do { \
        unlink(path); \
    } while (0)

#define CREATE_SYMLINK(target, link) \
    do { \
        symlink(target, link); \
    } while (0)

template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value << std::endl;
}

std::string parse_mac(const std::string& mac);
const char* resolve_symlink(const std::string& path);
