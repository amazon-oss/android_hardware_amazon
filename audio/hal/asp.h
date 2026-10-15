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

#define ASP_PARAMETER_MIC_PROCESSING "amazon_mic_processing"

int asp_send_command(int code, int value);
void asp_set_parameters(const char* kv_pairs);
audio_source_t remap_input_source(audio_source_t source);
