/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "audio.amazon_wrapper"

#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <log/log.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include <cutils/properties.h>

#include "audio_amazon.h"

static uint32_t stream_in_ext_get_sample_rate(const struct audio_stream* stream) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->get_sample_rate == NULL)
        return -ENOSYS;

    return common->get_sample_rate(common);
}

static int stream_in_ext_set_sample_rate(struct audio_stream* stream, uint32_t rate) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->set_sample_rate == NULL)
        return -ENOSYS;

    return common->set_sample_rate(common, rate);
}

static size_t stream_in_ext_get_buffer_size(const struct audio_stream* stream) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->get_buffer_size == NULL)
        return -ENOSYS;

    return common->get_buffer_size(common);
}

static audio_channel_mask_t stream_in_ext_get_channels(const struct audio_stream* stream) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->get_channels == NULL)
        return -ENOSYS;

    return common->get_channels(common);
}

static audio_format_t stream_in_ext_get_format(const struct audio_stream* stream) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->get_format == NULL)
        return AUDIO_FORMAT_INVALID;

    return common->get_format(common);
}

static int stream_in_ext_set_format(struct audio_stream* stream, audio_format_t format) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->set_format == NULL)
        return -ENOSYS;

    return common->set_format(common, format);
}

static int stream_in_ext_standby(struct audio_stream* stream) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->standby == NULL)
        return -ENOSYS;

    return common->standby(common);
}

static int stream_in_ext_dump(const struct audio_stream* stream, int fd) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->dump == NULL)
        return -ENOSYS;

    return common->dump(common, fd);
}

static int stream_in_ext_set_parameters(struct audio_stream* stream, const char* kvpairs) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->set_parameters == NULL)
        return -ENOSYS;

    return common->set_parameters(common, kvpairs);
}

static char* stream_in_ext_get_parameters(const struct audio_stream* stream,
                                             const char* keys) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->get_parameters == NULL)
        return NULL;

    return common->get_parameters(common, keys);
}

static int stream_in_ext_add_audio_effect(const struct audio_stream* stream,
                                          effect_handle_t effect) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->add_audio_effect == NULL)
        return -ENOSYS;

    return common->add_audio_effect(common, effect);
}

static int stream_in_ext_remove_audio_effect(const struct audio_stream* stream,
                                             effect_handle_t effect) {
    const struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<const struct audio_stream_in_ext*>(stream);
    const struct audio_stream* common = &stream_in_ext->legacy_stream->common;

    if (common->remove_audio_effect == NULL)
        return -ENOSYS;

    return common->remove_audio_effect(common, effect);
}

static int stream_in_ext_set_gain(struct audio_stream_in* stream, float gain) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct legacy_audio_stream_in* legacy_stream = stream_in_ext->legacy_stream;

    if (legacy_stream->set_gain == NULL)
        return -ENOSYS;

    return legacy_stream->set_gain(legacy_stream, gain);
}

static ssize_t stream_in_ext_read(struct audio_stream_in* stream, void* buffer,
                                     size_t bytes) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct legacy_audio_stream_in* legacy_stream = stream_in_ext->legacy_stream;

    if (legacy_stream->read == NULL)
        return -ENOSYS;

    return legacy_stream->read(legacy_stream, buffer, bytes);
}

static uint32_t stream_in_ext_get_input_frames_lost(struct audio_stream_in* stream) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(stream);
    struct legacy_audio_stream_in* legacy_stream = stream_in_ext->legacy_stream;

    if (legacy_stream->get_input_frames_lost == NULL)
        return -ENOSYS;

    return legacy_stream->get_input_frames_lost(legacy_stream);
}

static int stream_in_ext_get_capture_position(const struct audio_stream_in* stream,
                                              int64_t* frames, int64_t* time) {
    struct audio_stream_in_ext* stream_in_ext =
        reinterpret_cast<struct audio_stream_in_ext*>(const_cast<struct audio_stream_in*>(stream));
    struct legacy_audio_stream_in* legacy_stream = stream_in_ext->legacy_stream;

    if (legacy_stream->get_capture_position == NULL)
        return -ENOSYS;

    return legacy_stream->get_capture_position(legacy_stream, frames, time);
}

/* allocate and setup an audio_stream_in_ext */
struct audio_stream_in_ext* create_legacy_audio_stream_in_ext() {
    struct audio_stream_in_ext* stream_in_ext;
    struct audio_stream_in* stream;

    stream_in_ext = (struct audio_stream_in_ext*)calloc(1, sizeof(struct audio_stream_in_ext));
    if (!stream_in_ext) {
        ALOGE("Failed to allocate memory for audio_stream_in_ext");
        return NULL;
    }

    /* setup stream function pointers */
    stream = &stream_in_ext->stream_in;
    stream->common.get_sample_rate = stream_in_ext_get_sample_rate;
    stream->common.set_sample_rate = stream_in_ext_set_sample_rate;
    stream->common.get_buffer_size = stream_in_ext_get_buffer_size;
    stream->common.get_channels = stream_in_ext_get_channels;
    stream->common.get_format = stream_in_ext_get_format;
    stream->common.set_format = stream_in_ext_set_format;
    stream->common.standby = stream_in_ext_standby;
    stream->common.dump = stream_in_ext_dump;
    stream->common.set_parameters = stream_in_ext_set_parameters;
    stream->common.get_parameters = stream_in_ext_get_parameters;
    stream->common.add_audio_effect = stream_in_ext_add_audio_effect;
    stream->common.remove_audio_effect = stream_in_ext_remove_audio_effect;

    stream->set_gain = stream_in_ext_set_gain;
    stream->read = stream_in_ext_read;
    stream->get_input_frames_lost = stream_in_ext_get_input_frames_lost;

    /*
     * There is no real good way to know if the underlying HAL supports
     * get_capture_position, since we can't really know how much memory
     * it allocated for its legacy_audio_stream_in structure. Always trying calling
     * it may cause memory access violations as the memory for the function pointer
     * will probably not be zero-initialized (since the HAL allocates with calloc,
     * but using the sizeof the audio_stream_in structure that it was compiled against),
     * so even checking for NULL doesn't guarantee safety.
     */
    if (property_get_bool("ro.audiohal.has_get_capture_position", false))
        stream->get_capture_position = stream_in_ext_get_capture_position;

    return stream_in_ext;
}
