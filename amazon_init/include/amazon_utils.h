/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <iostream>

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

const char* resolve_symlink(const std::string& path);
