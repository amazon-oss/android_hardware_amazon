/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>
#include <errno.h>

#include "asp.h"

typedef int (*asp_command_t)(void* pipeline, int code, int size, void* data, void* reply,
                             int* reply_size);

static asp_command_t get_asp_command() {
    void* handle = dlopen("libasp.so", RTLD_NOW | RTLD_NOLOAD);

    return handle ? reinterpret_cast<asp_command_t>(dlsym(handle, "asp_command")) : nullptr;
}

int asp_send_command(int code, int value) {
    static const asp_command_t asp_command = get_asp_command();

    if (!asp_command)
        return -ENOSYS;

    return asp_command(nullptr, code, sizeof(value), &value, nullptr, nullptr);
}
