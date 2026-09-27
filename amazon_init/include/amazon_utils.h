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

template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value << std::endl;
}

std::string parse_mac(const std::string& mac);
std::string resolve_symlink(const std::string& path);
