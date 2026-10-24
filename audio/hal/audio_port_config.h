/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <system/audio.h>

/**
 * audio_port_config_device_ext as our audio HALs know it, from before
 * speaker_layout_channel_mask was added.
 */
struct amazon_audio_port_config_device_ext {
    audio_module_handle_t hw_module;
    audio_devices_t type;
    char address[AUDIO_DEVICE_MAX_ADDRESS_LEN];
};

/**
 * audio_port_config as our audio HALs know it, from before the flags field
 * was added in front of ext.
 */
struct amazon_audio_port_config {
    audio_port_handle_t id;
    audio_port_role_t role;
    audio_port_type_t type;
    unsigned int config_mask;
    unsigned int sample_rate;
    audio_channel_mask_t channel_mask;
    audio_format_t format;
    struct audio_gain_config gain;
    union {
        struct amazon_audio_port_config_device_ext device;
        struct audio_port_config_mix_ext mix;
        struct audio_port_config_session_ext session;
    } ext;
};
