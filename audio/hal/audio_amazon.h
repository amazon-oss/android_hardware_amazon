/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/audio.h>

#include "audio_port_config.h"

struct v28_audio_offload_info {
    uint16_t version;
    uint16_t size;
    uint32_t sample_rate;
    audio_channel_mask_t channel_mask;
    audio_format_t format;
    audio_stream_type_t stream_type;
    uint32_t bit_rate;
    int64_t duration_us;
    bool has_video;
    bool is_streaming;
    uint32_t bit_width;
    uint32_t offload_buffer_size;
    audio_usage_t usage;
} __attribute__((aligned(8)));

struct __attribute__((aligned(8))) v28_audio_config {
    uint32_t sample_rate;
    audio_channel_mask_t channel_mask;
    audio_format_t format;
    struct v28_audio_offload_info offload_info;
    uint32_t frame_count;
};

struct v28_audio_port_device_ext {
    audio_module_handle_t hw_module;
    audio_devices_t type;
    char address[AUDIO_DEVICE_MAX_ADDRESS_LEN];
};

struct v28_audio_port {
    audio_port_handle_t id;
    audio_port_role_t role;
    audio_port_type_t type;
    char name[AUDIO_PORT_MAX_NAME_LEN];
    unsigned int num_sample_rates;
    unsigned int sample_rates[AUDIO_PORT_MAX_SAMPLING_RATES];
    unsigned int num_channel_masks;
    audio_channel_mask_t channel_masks[AUDIO_PORT_MAX_CHANNEL_MASKS];
    unsigned int num_formats;
    audio_format_t formats[AUDIO_PORT_MAX_FORMATS];
    unsigned int num_gains;
    struct audio_gain gains[AUDIO_PORT_MAX_GAINS];
    struct amazon_audio_port_config active_config;
    union {
        struct v28_audio_port_device_ext device;
        struct audio_port_mix_ext mix;
        struct audio_port_session_ext session;
    } ext;
};

struct v28_record_track_metadata {
    audio_source_t source;
    float gain;
};

struct v28_sink_metadata {
    size_t track_count;
    struct v28_record_track_metadata* tracks;
};

struct v28_audio_stream_out {
    struct audio_stream common;
    uint32_t (*get_latency)(const struct v28_audio_stream_out* stream);
    int (*set_volume)(struct v28_audio_stream_out* stream, float left, float right);
    ssize_t (*write)(struct v28_audio_stream_out* stream, const void* buffer, size_t bytes);
    int (*get_render_position)(const struct v28_audio_stream_out* stream, uint32_t* dsp_frames);
    int (*get_next_write_timestamp)(const struct v28_audio_stream_out* stream, int64_t* timestamp);
    int (*set_callback)(struct v28_audio_stream_out* stream, stream_callback_t callback,
                        void* cookie);
    int (*pause)(struct v28_audio_stream_out* stream);
    int (*resume)(struct v28_audio_stream_out* stream);
    int (*drain)(struct v28_audio_stream_out* stream, audio_drain_type_t type);
    int (*flush)(struct v28_audio_stream_out* stream);
    int (*get_presentation_position)(const struct v28_audio_stream_out* stream, uint64_t* frames,
                                     struct timespec* timestamp);
    int (*start)(const struct v28_audio_stream_out* stream);
    int (*stop)(const struct v28_audio_stream_out* stream);
    int (*create_mmap_buffer)(const struct v28_audio_stream_out* stream, int32_t min_size_frames,
                              struct audio_mmap_buffer_info* info);
    int (*get_mmap_position)(const struct v28_audio_stream_out* stream,
                             struct audio_mmap_position* position);
    void (*update_source_metadata)(struct v28_audio_stream_out* stream,
                                   const struct source_metadata* source_metadata);
};

struct v28_audio_stream_in {
    struct audio_stream common;
    int (*set_gain)(struct v28_audio_stream_in* stream, float gain);
    ssize_t (*read)(struct v28_audio_stream_in* stream, void* buffer, size_t bytes);
    uint32_t (*get_input_frames_lost)(struct v28_audio_stream_in* stream);
    int (*get_capture_position)(const struct v28_audio_stream_in* stream, int64_t* frames,
                                int64_t* time);
    int (*start)(const struct v28_audio_stream_in* stream);
    int (*stop)(const struct v28_audio_stream_in* stream);
    int (*create_mmap_buffer)(const struct v28_audio_stream_in* stream, int32_t min_size_frames,
                              struct audio_mmap_buffer_info* info);
    int (*get_mmap_position)(const struct v28_audio_stream_in* stream,
                             struct audio_mmap_position* position);
    int (*get_active_microphones)(const struct v28_audio_stream_in* stream,
                                  struct audio_microphone_characteristic_t* mic_array,
                                  size_t* mic_count);
    void (*update_sink_metadata)(struct v28_audio_stream_in* stream,
                                 const struct v28_sink_metadata* sink_metadata);
};

struct v28_audio_hw_device {
    struct hw_device_t common;
    uint32_t (*get_supported_devices)(const struct v28_audio_hw_device* dev);
    int (*init_check)(const struct v28_audio_hw_device* dev);
    int (*set_voice_volume)(struct v28_audio_hw_device* dev, float volume);
    int (*set_master_volume)(struct v28_audio_hw_device* dev, float volume);
    int (*get_master_volume)(struct v28_audio_hw_device* dev, float* volume);
    int (*set_mode)(struct v28_audio_hw_device* dev, audio_mode_t mode);
    int (*set_mic_mute)(struct v28_audio_hw_device* dev, bool state);
    int (*get_mic_mute)(const struct v28_audio_hw_device* dev, bool* state);
    int (*set_parameters)(struct v28_audio_hw_device* dev, const char* kv_pairs);
    char* (*get_parameters)(const struct v28_audio_hw_device* dev, const char* keys);
    size_t (*get_input_buffer_size)(const struct v28_audio_hw_device* dev,
                                    const struct v28_audio_config* config);
    int (*open_output_stream)(struct v28_audio_hw_device* dev, audio_io_handle_t handle,
                              audio_devices_t devices, audio_output_flags_t flags,
                              struct v28_audio_config* config,
                              struct v28_audio_stream_out** stream_out, const char* address);
    void (*close_output_stream)(struct v28_audio_hw_device* dev,
                                struct v28_audio_stream_out* stream_out);
    int (*open_input_stream)(struct v28_audio_hw_device* dev, audio_io_handle_t handle,
                             audio_devices_t devices, struct v28_audio_config* config,
                             struct v28_audio_stream_in** stream_in, audio_input_flags_t flags,
                             const char* address, audio_source_t source);
    void (*close_input_stream)(struct v28_audio_hw_device* dev, struct v28_audio_stream_in* stream_in);
    int (*get_microphones)(const struct v28_audio_hw_device* dev,
                           struct audio_microphone_characteristic_t* mic_array, size_t* mic_count);
    int (*dump)(const struct v28_audio_hw_device* dev, int fd);
    int (*set_master_mute)(struct v28_audio_hw_device* dev, bool mute);
    int (*get_master_mute)(struct v28_audio_hw_device* dev, bool* mute);
    int (*create_audio_patch)(struct v28_audio_hw_device* dev, unsigned int num_sources,
                              const struct amazon_audio_port_config* sources,
                              unsigned int num_sinks, const struct amazon_audio_port_config* sinks,
                              audio_patch_handle_t* handle);
    int (*release_audio_patch)(struct v28_audio_hw_device* dev, audio_patch_handle_t handle);
    int (*get_audio_port)(struct v28_audio_hw_device* dev, struct v28_audio_port* port);
    int (*set_audio_port_config)(struct v28_audio_hw_device* dev,
                                 const struct amazon_audio_port_config* config);
};

struct v28_audio_stream_out_ext {
    struct audio_stream_out stream;
    struct v28_audio_stream_out* amazon_stream;
};

struct v28_audio_stream_in_ext {
    struct audio_stream_in stream;
    struct v28_audio_stream_in* amazon_stream;
};

struct v28_audio_stream_out_ext* create_v28_audio_stream_out_ext(
        struct v28_audio_stream_out* amazon_stream);
struct v28_audio_stream_in_ext* create_v28_audio_stream_in_ext(struct v28_audio_stream_in* amazon_stream);
