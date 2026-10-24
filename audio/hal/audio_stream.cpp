/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "audio.amazon_wrapper"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include <log/log.h>

#include <hardware/audio.h>
#include <system/audio.h>

#include "audio_amazon.h"

/*
 * Common stream methods
 */

template <typename T>
static struct audio_stream* to_amazon_common(const struct audio_stream* stream) {
    return &reinterpret_cast<const T*>(stream)->amazon_stream->common;
}

#define COMMON_METHODS(prefix, ext)                                                              \
    static uint32_t prefix##_get_sample_rate(const struct audio_stream* stream) {                \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->get_sample_rate(common);                                                  \
    }                                                                                            \
    static int prefix##_set_sample_rate(struct audio_stream* stream, uint32_t rate) {            \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->set_sample_rate(common, rate);                                            \
    }                                                                                            \
    static size_t prefix##_get_buffer_size(const struct audio_stream* stream) {                  \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->get_buffer_size(common);                                                  \
    }                                                                                            \
    static audio_channel_mask_t prefix##_get_channels(const struct audio_stream* stream) {       \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->get_channels(common);                                                     \
    }                                                                                            \
    static audio_format_t prefix##_get_format(const struct audio_stream* stream) {               \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->get_format(common);                                                       \
    }                                                                                            \
    static int prefix##_set_format(struct audio_stream* stream, audio_format_t format) {         \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->set_format(common, format);                                               \
    }                                                                                            \
    static int prefix##_dump(const struct audio_stream* stream, int fd) {                        \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->dump(common, fd);                                                         \
    }                                                                                            \
    static audio_devices_t prefix##_get_device(const struct audio_stream* stream) {              \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->get_device(common);                                                       \
    }                                                                                            \
    static int prefix##_set_device(struct audio_stream* stream, audio_devices_t device) {        \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->set_device(common, device);                                               \
    }                                                                                            \
    static int prefix##_set_parameters(struct audio_stream* stream, const char* kv_pairs) {      \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->set_parameters(common, kv_pairs);                                         \
    }                                                                                            \
    static char* prefix##_get_parameters(const struct audio_stream* stream, const char* keys) {  \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->get_parameters(common, keys);                                             \
    }                                                                                            \
    static int prefix##_add_audio_effect(const struct audio_stream* stream,                      \
                                         effect_handle_t effect) {                               \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->add_audio_effect(common, effect);                                         \
    }                                                                                            \
    static int prefix##_remove_audio_effect(const struct audio_stream* stream,                   \
                                            effect_handle_t effect) {                            \
        struct audio_stream* common = to_amazon_common<ext>(stream);                             \
        return common->remove_audio_effect(common, effect);                                      \
    }

/*
 * Output streams
 */

static struct v28_audio_stream_out* to_amazon(const struct audio_stream_out* stream) {
    return reinterpret_cast<const struct v28_audio_stream_out_ext*>(stream)->amazon_stream;
}

COMMON_METHODS(out, struct v28_audio_stream_out_ext)

static int out_standby(struct audio_stream* stream) {
    struct audio_stream* common = to_amazon_common<struct v28_audio_stream_out_ext>(stream);

    return common->standby(common);
}

static uint32_t out_get_latency(const struct audio_stream_out* stream) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->get_latency(amazon_stream);
}

static int out_set_volume(struct audio_stream_out* stream, float left, float right) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->set_volume(amazon_stream, left, right);
}

static ssize_t out_write(struct audio_stream_out* stream, const void* buffer, size_t bytes) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->write(amazon_stream, buffer, bytes);
}

static int out_get_render_position(const struct audio_stream_out* stream, uint32_t* dsp_frames) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->get_render_position(amazon_stream, dsp_frames);
}

static int out_get_next_write_timestamp(const struct audio_stream_out* stream, int64_t* timestamp) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->get_next_write_timestamp(amazon_stream, timestamp);
}

static int out_set_callback(struct audio_stream_out* stream, stream_callback_t callback,
                            void* cookie) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->set_callback(amazon_stream, callback, cookie);
}

static int out_pause(struct audio_stream_out* stream) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->pause(amazon_stream);
}

static int out_resume(struct audio_stream_out* stream) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->resume(amazon_stream);
}

static int out_drain(struct audio_stream_out* stream, audio_drain_type_t type) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->drain(amazon_stream, type);
}

static int out_flush(struct audio_stream_out* stream) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->flush(amazon_stream);
}

static int out_get_presentation_position(const struct audio_stream_out* stream, uint64_t* frames,
                                         struct timespec* timestamp) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->get_presentation_position(amazon_stream, frames, timestamp);
}

static int out_start(const struct audio_stream_out* stream) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->start(amazon_stream);
}

static int out_stop(const struct audio_stream_out* stream) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->stop(amazon_stream);
}

static int out_create_mmap_buffer(const struct audio_stream_out* stream, int32_t min_size_frames,
                                  struct audio_mmap_buffer_info* info) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    /* The HAL doesn't know about the flags at the end. */
    memset(info, 0, sizeof(*info));
    return amazon_stream->create_mmap_buffer(amazon_stream, min_size_frames, info);
}

static int out_get_mmap_position(const struct audio_stream_out* stream,
                                 struct audio_mmap_position* position) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    return amazon_stream->get_mmap_position(amazon_stream, position);
}

static void out_update_source_metadata(struct audio_stream_out* stream,
                                       const struct source_metadata* source_metadata) {
    struct v28_audio_stream_out* amazon_stream = to_amazon(stream);

    amazon_stream->update_source_metadata(amazon_stream, source_metadata);
}

#define FORWARD(field) out->stream.field = amazon_stream->field ? out_##field : nullptr
#define FORWARD_COMMON(field) \
    out->stream.common.field = amazon_stream->common.field ? out_##field : nullptr

struct v28_audio_stream_out_ext* create_v28_audio_stream_out_ext(
        struct v28_audio_stream_out* amazon_stream) {
    struct v28_audio_stream_out_ext* out;

    out = (struct v28_audio_stream_out_ext*)calloc(1, sizeof(struct v28_audio_stream_out_ext));
    if (!out) {
        ALOGE("Failed to allocate memory for v28_audio_stream_out_ext");
        return NULL;
    }

    out->amazon_stream = amazon_stream;

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

    FORWARD(get_latency);
    FORWARD(set_volume);
    FORWARD(write);
    FORWARD(get_render_position);
    FORWARD(get_next_write_timestamp);
    FORWARD(set_callback);
    FORWARD(pause);
    FORWARD(resume);
    FORWARD(drain);
    FORWARD(flush);
    FORWARD(get_presentation_position);
    FORWARD(start);
    FORWARD(stop);
    FORWARD(create_mmap_buffer);
    FORWARD(get_mmap_position);
    FORWARD(update_source_metadata);

    return out;
}

#undef FORWARD
#undef FORWARD_COMMON

/*
 * Input streams
 */

static struct v28_audio_stream_in* to_amazon(const struct audio_stream_in* stream) {
    return reinterpret_cast<const struct v28_audio_stream_in_ext*>(stream)->amazon_stream;
}

COMMON_METHODS(in, struct v28_audio_stream_in_ext)

static int in_standby(struct audio_stream* stream) {
    struct audio_stream* common = to_amazon_common<struct v28_audio_stream_in_ext>(stream);

    return common->standby(common);
}

static int in_set_gain(struct audio_stream_in* stream, float gain) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->set_gain(amazon_stream, gain);
}

static ssize_t in_read(struct audio_stream_in* stream, void* buffer, size_t bytes) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->read(amazon_stream, buffer, bytes);
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in* stream) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_input_frames_lost(amazon_stream);
}

static int in_get_capture_position(const struct audio_stream_in* stream, int64_t* frames,
                                   int64_t* time) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_capture_position(amazon_stream, frames, time);
}

static int in_start(const struct audio_stream_in* stream) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->start(amazon_stream);
}

static int in_stop(const struct audio_stream_in* stream) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->stop(amazon_stream);
}

static int in_create_mmap_buffer(const struct audio_stream_in* stream, int32_t min_size_frames,
                                 struct audio_mmap_buffer_info* info) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    /* The HAL doesn't know about the flags at the end. */
    memset(info, 0, sizeof(*info));
    return amazon_stream->create_mmap_buffer(amazon_stream, min_size_frames, info);
}

static int in_get_mmap_position(const struct audio_stream_in* stream,
                                struct audio_mmap_position* position) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_mmap_position(amazon_stream, position);
}

static int in_get_active_microphones(const struct audio_stream_in* stream,
                                     struct audio_microphone_characteristic_t* mic_array,
                                     size_t* mic_count) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);

    return amazon_stream->get_active_microphones(amazon_stream, mic_array, mic_count);
}

static void in_update_sink_metadata(struct audio_stream_in* stream,
                                    const struct sink_metadata* sink_metadata) {
    struct v28_audio_stream_in* amazon_stream = to_amazon(stream);
    std::vector<struct v28_record_track_metadata> tracks(sink_metadata->track_count);
    struct v28_sink_metadata amazon_metadata = {
            .track_count = tracks.size(),
            .tracks = tracks.data(),
    };

    for (size_t i = 0; i < tracks.size(); i++) {
        tracks[i].source = sink_metadata->tracks[i].source;
        tracks[i].gain = sink_metadata->tracks[i].gain;
    }

    amazon_stream->update_sink_metadata(amazon_stream, &amazon_metadata);
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
