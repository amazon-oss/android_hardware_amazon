/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "audio.amazon_wrapper"

#include <stddef.h>
#include <stdlib.h>

#include <log/log.h>

#include <hardware/audio.h>
#include <system/audio.h>

#include "asp.h"
#include "audio_amazon.h"

static_assert(offsetof(struct v28_audio_stream_in, get_active_microphones) ==
                      offsetof(struct audio_stream_in, get_active_microphones),
              "v28_audio_stream_in must match audio_stream_in up to get_active_microphones");

static struct v28_audio_stream_in* to_amazon(const struct audio_stream* stream) {
    return reinterpret_cast<const struct v28_audio_stream_in_ext*>(stream)->amazon_stream;
}

static struct v28_audio_stream_in* to_amazon(const struct v28_audio_stream_in* stream) {
    return reinterpret_cast<const struct v28_audio_stream_in_ext*>(stream)->amazon_stream;
}

static uint32_t in_get_sample_rate(const struct audio_stream* stream) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->get_sample_rate(common);
}

static int in_set_sample_rate(struct audio_stream* stream, uint32_t rate) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->set_sample_rate(common, rate);
}

static size_t in_get_buffer_size(const struct audio_stream* stream) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->get_buffer_size(common);
}

static audio_channel_mask_t in_get_channels(const struct audio_stream* stream) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->get_channels(common);
}

static audio_format_t in_get_format(const struct audio_stream* stream) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->get_format(common);
}

static int in_set_format(struct audio_stream* stream, audio_format_t format) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->set_format(common, format);
}

static int in_standby(struct audio_stream* stream) {
    struct v28_audio_stream_in_ext* in = reinterpret_cast<struct v28_audio_stream_in_ext*>(stream);
    struct audio_stream* common = &in->amazon_stream->common;

    in->capture_started = false;
    return common->standby(common);
}

static int in_dump(const struct audio_stream* stream, int fd) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->dump(common, fd);
}

static audio_devices_t in_get_device(const struct audio_stream* stream) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->get_device(common);
}

static int in_set_device(struct audio_stream* stream, audio_devices_t device) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->set_device(common, device);
}

static int in_set_parameters(struct audio_stream* stream, const char* kv_pairs) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->set_parameters(common, kv_pairs);
}

static char* in_get_parameters(const struct audio_stream* stream, const char* keys) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->get_parameters(common, keys);
}

static int in_add_audio_effect(const struct audio_stream* stream, effect_handle_t effect) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->add_audio_effect(common, effect);
}

static int in_remove_audio_effect(const struct audio_stream* stream, effect_handle_t effect) {
    struct audio_stream* common = &to_amazon(stream)->common;

    return common->remove_audio_effect(common, effect);
}

static int in_set_gain(struct v28_audio_stream_in* stream, float gain) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->set_gain(amazon_stream, gain);
}

static ssize_t in_read(struct v28_audio_stream_in* stream, void* buffer, size_t bytes) {
    struct v28_audio_stream_in_ext* in = reinterpret_cast<struct v28_audio_stream_in_ext*>(stream);
    ssize_t ret = in->amazon_stream->read(in->amazon_stream, buffer, bytes);

    if (in->mic_capture && !in->capture_started) {
        asp_send_command(ASP_CMD_SET_VOIP_MODE, 0);
        in->capture_started = true;
    }

    return ret;
}

static uint32_t in_get_input_frames_lost(struct v28_audio_stream_in* stream) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_input_frames_lost(amazon_stream);
}

static int in_get_capture_position(const struct v28_audio_stream_in* stream, int64_t* frames,
                                   int64_t* time) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_capture_position(amazon_stream, frames, time);
}

static int in_start(const struct v28_audio_stream_in* stream) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->start(amazon_stream);
}

static int in_stop(const struct v28_audio_stream_in* stream) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->stop(amazon_stream);
}

static int in_create_mmap_buffer(const struct v28_audio_stream_in* stream, int32_t min_size_frames,
                                 struct audio_mmap_buffer_info* info) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->create_mmap_buffer(amazon_stream, min_size_frames, info);
}

static int in_get_mmap_position(const struct v28_audio_stream_in* stream,
                                struct audio_mmap_position* position) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_mmap_position(amazon_stream, position);
}

static int in_get_active_microphones(const struct v28_audio_stream_in* stream,
                                     struct audio_microphone_characteristic_t* mic_array,
                                     size_t* mic_count) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_active_microphones(amazon_stream, mic_array, mic_count);
}

static void in_update_sink_metadata(struct v28_audio_stream_in* stream,
                                    const struct sink_metadata* sink_metadata) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    amazon_stream->update_sink_metadata(amazon_stream, sink_metadata);
}

#define FORWARD(field) in->stream.field = amazon_stream->field ? in_##field : nullptr
#define FORWARD_COMMON(field) \
    in->stream.common.field = amazon_stream->common.field ? in_##field : nullptr

struct v28_audio_stream_in_ext* create_v28_audio_stream_in_ext(
        struct v28_audio_stream_in* amazon_stream) {
    struct v28_audio_stream_in_ext* in;

    in = (struct v28_audio_stream_in_ext*)calloc(1, sizeof(struct v28_audio_stream_in_ext));
    if (!in) {
        ALOGE("Failed to allocate memory for v28_audio_stream_in_ext");
        return NULL;
    }

    in->amazon_stream = amazon_stream;

    FORWARD_COMMON(get_sample_rate);
    FORWARD_COMMON(set_sample_rate);
    FORWARD_COMMON(get_buffer_size);
    FORWARD_COMMON(get_channels);
    FORWARD_COMMON(get_format);
    FORWARD_COMMON(set_format);
    FORWARD_COMMON(standby);
    FORWARD_COMMON(dump);
    FORWARD_COMMON(get_device);
    FORWARD_COMMON(set_device);
    FORWARD_COMMON(set_parameters);
    FORWARD_COMMON(get_parameters);
    FORWARD_COMMON(add_audio_effect);
    FORWARD_COMMON(remove_audio_effect);

    FORWARD(set_gain);
    FORWARD(read);
    FORWARD(get_input_frames_lost);
    FORWARD(get_capture_position);
    FORWARD(start);
    FORWARD(stop);
    FORWARD(create_mmap_buffer);
    FORWARD(get_mmap_position);
    FORWARD(get_active_microphones);
    FORWARD(update_sink_metadata);

    return in;
}
