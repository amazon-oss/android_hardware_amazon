/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/audio.h>

#include "audio_amazon.h"

struct p_audio_stream_in {
    struct audio_stream common;
    int (*set_gain)(struct p_audio_stream_in* stream, float gain);
    ssize_t (*read)(struct p_audio_stream_in* stream, void* buffer, size_t bytes);
    uint32_t (*get_input_frames_lost)(struct p_audio_stream_in* stream);
    int (*get_capture_position)(const struct p_audio_stream_in* stream, int64_t* frames,
                                int64_t* time);
    int (*start)(const struct p_audio_stream_in* stream);
    int (*stop)(const struct p_audio_stream_in* stream);
    int (*create_mmap_buffer)(const struct p_audio_stream_in* stream, int32_t min_size_frames,
                              struct audio_mmap_buffer_info* info);
    int (*get_mmap_position)(const struct p_audio_stream_in* stream,
                             struct audio_mmap_position* position);
    int (*get_active_microphones)(const struct p_audio_stream_in* stream,
                                  struct audio_microphone_characteristic_t* mic_array,
                                  size_t* mic_count);
    void (*update_sink_metadata)(struct p_audio_stream_in* stream,
                                 const struct sink_metadata* sink_metadata);
};

struct p_audio_hw_device {
    struct hw_device_t common;
    uint32_t (*get_supported_devices)(const struct p_audio_hw_device* dev);
    int (*init_check)(const struct p_audio_hw_device* dev);
    int (*set_voice_volume)(struct p_audio_hw_device* dev, float volume);
    int (*set_master_volume)(struct p_audio_hw_device* dev, float volume);
    int (*get_master_volume)(struct p_audio_hw_device* dev, float* volume);
    int (*set_mode)(struct p_audio_hw_device* dev, audio_mode_t mode);
    int (*set_mic_mute)(struct p_audio_hw_device* dev, bool state);
    int (*get_mic_mute)(const struct p_audio_hw_device* dev, bool* state);
    int (*set_parameters)(struct p_audio_hw_device* dev, const char* kv_pairs);
    char* (*get_parameters)(const struct p_audio_hw_device* dev, const char* keys);
    size_t (*get_input_buffer_size)(const struct p_audio_hw_device* dev,
                                    const struct audio_config* config);
    int (*open_output_stream)(struct p_audio_hw_device* dev, audio_io_handle_t handle,
                              audio_devices_t devices, audio_output_flags_t flags,
                              struct audio_config* config, struct audio_stream_out** stream_out,
                              const char* address);
    void (*close_output_stream)(struct p_audio_hw_device* dev, struct audio_stream_out* stream_out);
    int (*open_input_stream)(struct p_audio_hw_device* dev, audio_io_handle_t handle,
                             audio_devices_t devices, struct audio_config* config,
                             struct p_audio_stream_in** stream_in, audio_input_flags_t flags,
                             const char* address, audio_source_t source);
    void (*close_input_stream)(struct p_audio_hw_device* dev, struct p_audio_stream_in* stream_in);
    int (*get_microphones)(const struct p_audio_hw_device* dev,
                           struct audio_microphone_characteristic_t* mic_array, size_t* mic_count);
    int (*dump)(const struct p_audio_hw_device* dev, int fd);
    int (*set_master_mute)(struct p_audio_hw_device* dev, bool mute);
    int (*get_master_mute)(struct p_audio_hw_device* dev, bool* mute);
    int (*create_audio_patch)(struct p_audio_hw_device* dev, unsigned int num_sources,
                              const struct amazon_audio_port_config* sources,
                              unsigned int num_sinks, const struct amazon_audio_port_config* sinks,
                              audio_patch_handle_t* handle);
    int (*release_audio_patch)(struct p_audio_hw_device* dev, audio_patch_handle_t handle);
    int (*get_audio_port)(struct p_audio_hw_device* dev, struct audio_port* port);
    int (*set_audio_port_config)(struct p_audio_hw_device* dev,
                                 const struct amazon_audio_port_config* config);
};

struct p_audio_stream_in_ext {
    struct p_audio_stream_in stream;
    struct p_audio_stream_in* amazon_stream;
    bool mic_capture;
    bool capture_started;
};

struct p_audio_stream_in_ext* create_p_audio_stream_in_ext(struct p_audio_stream_in* amazon_stream);
