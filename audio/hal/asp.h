/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <system/audio.h>

#define ASP_CMD_SET_VOIP_MODE 3
#define ASP_CMD_SET_VOLUME 5
#define ASP_CMD_NOTIFY_VOICE_VOLUME 101

#define ASP_VOLUME_MAX 30

int asp_send_command(int code, int value);

/*
 * The HAL only runs Amazon's capture processing for voice sources, there is
 * no pipeline for plain mic captures. Use the voice one, it has AGC and NR.
 */
static inline audio_source_t remap_input_source(audio_source_t source) {
    switch (source) {
        case AUDIO_SOURCE_MIC:
        case AUDIO_SOURCE_CAMCORDER:
            return AUDIO_SOURCE_VOICE_COMMUNICATION;
        default:
            return source;
    }
}
