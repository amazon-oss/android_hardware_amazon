/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include <atomic>

#include <cutils/str_parms.h>

#include "asp.h"

static std::atomic<bool> noise_suppression(true);

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

void asp_set_parameters(const char* kv_pairs) {
    struct str_parms* parms = str_parms_create_str(kv_pairs);
    char value[8];

    if (!parms)
        return;

    if (str_parms_get_str(parms, ASP_PARAMETER_NOISE_SUPPRESSION, value, sizeof(value)) >= 0)
        noise_suppression = strcmp(value, "off") != 0;

    str_parms_destroy(parms);
}

/*
 * The HAL only runs Amazon's capture processing for voice sources, there is
 * no pipeline for plain mic captures. Use the voice one, it has AGC and NR.
 */
audio_source_t remap_input_source(audio_source_t source) {
    if (!noise_suppression)
        return source;

    switch (source) {
        case AUDIO_SOURCE_MIC:
        case AUDIO_SOURCE_CAMCORDER:
            return AUDIO_SOURCE_VOICE_COMMUNICATION;
        default:
            return source;
    }
}
