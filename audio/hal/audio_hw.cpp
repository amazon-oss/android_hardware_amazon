/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "audio.amazon_wrapper"

#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include "asp.h"
#include "audio_amazon.h"

struct v28_wrapper_audio_device {
    struct audio_hw_device device;
    struct v28_audio_hw_device* amazon_device;
};

static_assert(offsetof(struct amazon_audio_port_config, gain) ==
                      offsetof(struct audio_port_config, gain),
              "amazon_audio_port_config must match audio_port_config up to gain");

static struct v28_audio_hw_device* to_amazon(const struct audio_hw_device* dev) {
    return reinterpret_cast<const struct v28_wrapper_audio_device*>(dev)->amazon_device;
}

static void to_amazon_config(const struct audio_config* config,
                             struct v28_audio_config* amazon_config) {
    const audio_offload_info_t* info = &config->offload_info;
    struct v28_audio_offload_info* amazon_info = &amazon_config->offload_info;

    memset(amazon_config, 0, sizeof(*amazon_config));
    amazon_config->sample_rate = config->sample_rate;
    amazon_config->channel_mask = config->channel_mask;
    amazon_config->format = config->format;
    amazon_config->frame_count = config->frame_count;

    amazon_info->version = AUDIO_MAKE_OFFLOAD_INFO_VERSION(0, 1);
    amazon_info->size = sizeof(*amazon_info);
    amazon_info->sample_rate = info->sample_rate;
    amazon_info->channel_mask = info->channel_mask;
    amazon_info->format = info->format;
    amazon_info->stream_type = info->stream_type;
    amazon_info->bit_rate = info->bit_rate;
    amazon_info->duration_us = info->duration_us;
    amazon_info->has_video = info->has_video;
    amazon_info->is_streaming = info->is_streaming;
    amazon_info->bit_width = info->bit_width;
    amazon_info->offload_buffer_size = info->offload_buffer_size;
    amazon_info->usage = info->usage;
}

static void from_amazon_config(const struct v28_audio_config* amazon_config,
                               struct audio_config* config) {
    config->sample_rate = amazon_config->sample_rate;
    config->channel_mask = amazon_config->channel_mask;
    config->format = amazon_config->format;
    config->frame_count = amazon_config->frame_count;
}

static void to_amazon_port_config(const struct audio_port_config* config,
                                  struct amazon_audio_port_config* amazon_config) {
    memset(amazon_config, 0, sizeof(*amazon_config));
    memcpy(amazon_config, config, offsetof(struct amazon_audio_port_config, ext));

    switch (config->type) {
        case AUDIO_PORT_TYPE_DEVICE:
            amazon_config->ext.device.hw_module = config->ext.device.hw_module;
            amazon_config->ext.device.type = config->ext.device.type;
            memcpy(amazon_config->ext.device.address, config->ext.device.address,
                   sizeof(amazon_config->ext.device.address));
            break;
        case AUDIO_PORT_TYPE_MIX:
            amazon_config->ext.mix = config->ext.mix;
            break;
        case AUDIO_PORT_TYPE_SESSION:
            amazon_config->ext.session = config->ext.session;
            break;
        default:
            break;
    }
}

static void from_amazon_port_config(const struct amazon_audio_port_config* amazon_config,
                                    struct audio_port_config* config) {
    memset(config, 0, sizeof(*config));
    memcpy(config, amazon_config, offsetof(struct amazon_audio_port_config, ext));

    switch (amazon_config->type) {
        case AUDIO_PORT_TYPE_DEVICE:
            config->ext.device.hw_module = amazon_config->ext.device.hw_module;
            config->ext.device.type = amazon_config->ext.device.type;
            memcpy(config->ext.device.address, amazon_config->ext.device.address,
                   sizeof(amazon_config->ext.device.address));
            break;
        case AUDIO_PORT_TYPE_MIX:
            config->ext.mix = amazon_config->ext.mix;
            break;
        case AUDIO_PORT_TYPE_SESSION:
            config->ext.session = amazon_config->ext.session;
            break;
        default:
            break;
    }
}

static void to_amazon_port(const struct audio_port* port, struct v28_audio_port* amazon_port) {
    memset(amazon_port, 0, sizeof(*amazon_port));
    memcpy(amazon_port, port, offsetof(struct v28_audio_port, active_config));
    to_amazon_port_config(&port->active_config, &amazon_port->active_config);

    switch (port->type) {
        case AUDIO_PORT_TYPE_DEVICE:
            amazon_port->ext.device.hw_module = port->ext.device.hw_module;
            amazon_port->ext.device.type = port->ext.device.type;
            memcpy(amazon_port->ext.device.address, port->ext.device.address,
                   sizeof(amazon_port->ext.device.address));
            break;
        case AUDIO_PORT_TYPE_MIX:
            amazon_port->ext.mix = port->ext.mix;
            break;
        case AUDIO_PORT_TYPE_SESSION:
            amazon_port->ext.session = port->ext.session;
            break;
        default:
            break;
    }
}

static void from_amazon_port(const struct v28_audio_port* amazon_port, struct audio_port* port) {
    memset(port, 0, sizeof(*port));
    memcpy(port, amazon_port, offsetof(struct v28_audio_port, active_config));
    from_amazon_port_config(&amazon_port->active_config, &port->active_config);

    switch (amazon_port->type) {
        case AUDIO_PORT_TYPE_DEVICE:
            port->ext.device.hw_module = amazon_port->ext.device.hw_module;
            port->ext.device.type = amazon_port->ext.device.type;
            memcpy(port->ext.device.address, amazon_port->ext.device.address,
                   sizeof(amazon_port->ext.device.address));
            break;
        case AUDIO_PORT_TYPE_MIX:
            port->ext.mix = amazon_port->ext.mix;
            break;
        case AUDIO_PORT_TYPE_SESSION:
            port->ext.session = amazon_port->ext.session;
            break;
        default:
            break;
    }
}

static uint32_t adev_get_supported_devices(const struct audio_hw_device* dev) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->get_supported_devices(adev);
}

static int adev_init_check(const struct audio_hw_device* dev) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->init_check(adev);
}

static int adev_set_voice_volume(struct audio_hw_device* dev, float volume) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    int rc = adev->set_voice_volume(adev, volume);

    asp_send_command(ASP_CMD_NOTIFY_VOICE_VOLUME, lroundf(volume * 100));
    return rc;
}

static int adev_set_master_volume(struct audio_hw_device* dev, float volume) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->set_master_volume(adev, volume);
}

static int adev_get_master_volume(struct audio_hw_device* dev, float* volume) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->get_master_volume(adev, volume);
}

static int adev_set_mode(struct audio_hw_device* dev, audio_mode_t mode) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->set_mode(adev, mode);
}

static int adev_set_mic_mute(struct audio_hw_device* dev, bool state) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->set_mic_mute(adev, state);
}

static int adev_get_mic_mute(const struct audio_hw_device* dev, bool* state) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->get_mic_mute(adev, state);
}

static int adev_set_parameters(struct audio_hw_device* dev, const char* kv_pairs) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->set_parameters(adev, kv_pairs);
}

static char* adev_get_parameters(const struct audio_hw_device* dev, const char* keys) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->get_parameters(adev, keys);
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device* dev,
                                         const struct audio_config* config) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct v28_audio_config amazon_config;

    to_amazon_config(config, &amazon_config);
    return adev->get_input_buffer_size(adev, &amazon_config);
}

static int adev_open_output_stream(struct audio_hw_device* dev, audio_io_handle_t handle,
                                   audio_devices_t devices, audio_output_flags_t flags,
                                   struct audio_config* config,
                                   struct audio_stream_out** stream_out, const char* address) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct v28_audio_stream_out* amazon_stream = nullptr;
    struct v28_audio_stream_out_ext* out;
    struct v28_audio_config amazon_config;
    int rc;

    to_amazon_config(config, &amazon_config);
    rc = adev->open_output_stream(adev, handle, devices, flags, &amazon_config, &amazon_stream,
                                  address);
    from_amazon_config(&amazon_config, config);
    if (rc)
        return rc;

    out = create_v28_audio_stream_out_ext(amazon_stream);
    if (!out) {
        adev->close_output_stream(adev, amazon_stream);
        return -ENOMEM;
    }

    *stream_out = &out->stream;
    return 0;
}

static void adev_close_output_stream(struct audio_hw_device* dev,
                                     struct audio_stream_out* stream_out) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct v28_audio_stream_out_ext* out =
            reinterpret_cast<struct v28_audio_stream_out_ext*>(stream_out);

    adev->close_output_stream(adev, out->amazon_stream);
    free(out);
}

static int adev_open_input_stream(struct audio_hw_device* dev, audio_io_handle_t handle,
                                  audio_devices_t devices, struct audio_config* config,
                                  struct audio_stream_in** stream_in, audio_input_flags_t flags,
                                  const char* address, audio_source_t source) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct v28_audio_stream_in* amazon_stream = nullptr;
    struct v28_audio_stream_in_ext* in;
    struct v28_audio_config amazon_config;
    int rc;

    to_amazon_config(config, &amazon_config);
    rc = adev->open_input_stream(adev, handle, devices, &amazon_config, &amazon_stream, flags,
                                 address, source);
    from_amazon_config(&amazon_config, config);
    if (rc)
        return rc;

    in = create_v28_audio_stream_in_ext(amazon_stream);
    if (!in) {
        adev->close_input_stream(adev, amazon_stream);
        return -ENOMEM;
    }

    if (source == AUDIO_SOURCE_VOICE_COMMUNICATION)
        asp_send_command(ASP_CMD_SET_VOLUME, ASP_VOLUME_MAX);

    *stream_in = &in->stream;
    return 0;
}

static void adev_close_input_stream(struct audio_hw_device* dev,
                                    struct audio_stream_in* stream_in) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct v28_audio_stream_in_ext* in = reinterpret_cast<struct v28_audio_stream_in_ext*>(stream_in);

    adev->close_input_stream(adev, in->amazon_stream);
    free(in);
}

static int adev_get_microphones(const struct audio_hw_device* dev,
                                struct audio_microphone_characteristic_t* mic_array,
                                size_t* mic_count) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->get_microphones(adev, mic_array, mic_count);
}

static int adev_dump(const struct audio_hw_device* dev, int fd) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->dump(adev, fd);
}

static int adev_set_master_mute(struct audio_hw_device* dev, bool mute) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->set_master_mute(adev, mute);
}

static int adev_get_master_mute(struct audio_hw_device* dev, bool* mute) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->get_master_mute(adev, mute);
}

static int adev_create_audio_patch(struct audio_hw_device* dev, unsigned int num_sources,
                                   const struct audio_port_config* sources,
                                   unsigned int num_sinks, const struct audio_port_config* sinks,
                                   audio_patch_handle_t* handle) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct amazon_audio_port_config amazon_sources[AUDIO_PATCH_PORTS_MAX];
    struct amazon_audio_port_config amazon_sinks[AUDIO_PATCH_PORTS_MAX];

    if (num_sources > AUDIO_PATCH_PORTS_MAX || num_sinks > AUDIO_PATCH_PORTS_MAX)
        return -EINVAL;

    for (unsigned int i = 0; i < num_sources; i++)
        to_amazon_port_config(&sources[i], &amazon_sources[i]);

    for (unsigned int i = 0; i < num_sinks; i++)
        to_amazon_port_config(&sinks[i], &amazon_sinks[i]);

    return adev->create_audio_patch(adev, num_sources, amazon_sources, num_sinks, amazon_sinks,
                                    handle);
}

static int adev_release_audio_patch(struct audio_hw_device* dev, audio_patch_handle_t handle) {
    struct v28_audio_hw_device* adev = to_amazon(dev);

    return adev->release_audio_patch(adev, handle);
}

static int adev_get_audio_port(struct audio_hw_device* dev, struct audio_port* port) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct v28_audio_port amazon_port;
    int rc;

    if (!adev->get_audio_port)
        return -ENOSYS;

    to_amazon_port(port, &amazon_port);
    rc = adev->get_audio_port(adev, &amazon_port);
    if (!rc)
        from_amazon_port(&amazon_port, port);

    return rc;
}

static int adev_set_audio_port_config(struct audio_hw_device* dev,
                                      const struct audio_port_config* config) {
    struct v28_audio_hw_device* adev = to_amazon(dev);
    struct amazon_audio_port_config amazon_config;

    to_amazon_port_config(config, &amazon_config);
    return adev->set_audio_port_config(adev, &amazon_config);
}

static int adev_close(hw_device_t* device) {
    struct v28_wrapper_audio_device* ctx = reinterpret_cast<struct v28_wrapper_audio_device*>(device);

    ctx->amazon_device->common.close(&ctx->amazon_device->common);
    free(ctx);

    return 0;
}

#define FORWARD(field) adev->device.field = adev->amazon_device->field ? adev_##field : nullptr

static int adev_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    struct v28_wrapper_audio_device* adev;
    const struct hw_module_t* amazon_module;
    int rc;

    ALOGI("adev_open: %s", name);

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    rc = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID, "primary_amazon", &amazon_module);
    if (rc) {
        ALOGE("Could not get audio hw module");
        return rc;
    }

    adev = (struct v28_wrapper_audio_device*)calloc(1, sizeof(struct v28_wrapper_audio_device));
    if (!adev)
        return -ENOMEM;

    rc = amazon_module->methods->open(amazon_module, name,
                                      (struct hw_device_t**)&adev->amazon_device);
    if (rc) {
        ALOGE("Could not open amazon audio hw device");
        free(adev);
        return rc;
    }

    adev->device.common.tag = HARDWARE_DEVICE_TAG;
    adev->device.common.version = adev->amazon_device->common.version;
    adev->device.common.module = (struct hw_module_t*)module;
    adev->device.common.close = adev_close;

    FORWARD(get_supported_devices);
    FORWARD(init_check);
    FORWARD(set_voice_volume);
    FORWARD(set_master_volume);
    FORWARD(get_master_volume);
    FORWARD(set_mode);
    FORWARD(set_mic_mute);
    FORWARD(get_mic_mute);
    FORWARD(set_parameters);
    FORWARD(get_parameters);
    FORWARD(get_input_buffer_size);
    FORWARD(open_output_stream);
    FORWARD(close_output_stream);
    FORWARD(open_input_stream);
    FORWARD(close_input_stream);
    FORWARD(get_microphones);
    FORWARD(dump);
    FORWARD(set_master_mute);
    FORWARD(get_master_mute);
    FORWARD(create_audio_patch);
    FORWARD(release_audio_patch);
    FORWARD(set_audio_port_config);

    /* The HIDL HAL calls this without checking it, the wrapper handles NULL. */
    adev->device.get_audio_port = adev_get_audio_port;

    *device = &adev->device.common;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
        .open = adev_open,
};

// clang-format off
extern "C" struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Amazon audio HAL wrapper",
        .author = "The LineageOS Project",
        .methods = &hal_module_methods,
    },
};
// clang-format on
