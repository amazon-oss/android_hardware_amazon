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

static std::atomic<audio_source_t> mic_source(AUDIO_SOURCE_VOICE_COMMUNICATION);

typedef int (*asp_command_t)(void* pipeline, int code, int size, void* data, void* reply,
                             int* reply_size);

static asp_command_t get_asp_command() {
    static const asp_command_t asp_command = [] {
        void* handle = dlopen("libasp.so", RTLD_NOW | RTLD_NOLOAD);

        return handle ? reinterpret_cast<asp_command_t>(dlsym(handle, "asp_command")) : nullptr;
    }();

    return asp_command;
}

int asp_send_command(int code, int value) {
    asp_command_t asp_command = get_asp_command();

    if (!asp_command)
        return -ENOSYS;

    return asp_command(nullptr, code, sizeof(value), &value, nullptr, nullptr);
}

void asp_set_parameters(const char* kv_pairs) {
    struct str_parms* parms = str_parms_create_str(kv_pairs);
    char value[16];

    if (!parms)
        return;

    if (str_parms_get_str(parms, ASP_PARAMETER_MIC_PROCESSING, value, sizeof(value)) >= 0) {
        if (!strcmp(value, "far_field"))
            mic_source = AUDIO_SOURCE_VOICE_RECOGNITION;
        else if (!strcmp(value, "off"))
            mic_source = AUDIO_SOURCE_DEFAULT;
        else
            mic_source = AUDIO_SOURCE_VOICE_COMMUNICATION;
    }

    str_parms_destroy(parms);
}

/*
 * The HAL only runs Amazon's capture processing for voice sources, there is
 * no pipeline for plain mic captures. Use the voice one by default, it has
 * AGC and NR. The ASR one beamforms all mics but leaves AGC and NR to the app.
 */
audio_source_t remap_input_source(audio_source_t source) {
    audio_source_t target = mic_source;

    if (target == AUDIO_SOURCE_DEFAULT || !get_asp_command())
        return source;

    switch (source) {
        case AUDIO_SOURCE_MIC:
        case AUDIO_SOURCE_CAMCORDER:
            return target;
        default:
            return source;
    }
}
