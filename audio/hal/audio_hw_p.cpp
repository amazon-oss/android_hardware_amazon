/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "audio.amazon_wrapper"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>

#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <system/audio.h>

#include "asp.h"
#include "audio_amazon_p.h"

struct p_wrapper_audio_device {
    struct p_audio_hw_device device;
    struct p_audio_hw_device* amazon_device;
};

static_assert(offsetof(struct p_audio_hw_device, set_audio_port_config) ==
                      offsetof(struct audio_hw_device, set_audio_port_config),
              "p_audio_hw_device must match audio_hw_device up to set_audio_port_config");

static struct p_audio_hw_device* to_amazon(const struct p_audio_hw_device* dev) {
    return reinterpret_cast<const struct p_wrapper_audio_device*>(dev)->amazon_device;
}

static uint32_t adev_get_supported_devices(const struct p_audio_hw_device* dev) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_supported_devices(adev);
}

static int adev_init_check(const struct p_audio_hw_device* dev) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->init_check(adev);
}

static int adev_set_voice_volume(struct p_audio_hw_device* dev, float volume) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_voice_volume(adev, volume);
}

static int adev_set_master_volume(struct p_audio_hw_device* dev, float volume) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_master_volume(adev, volume);
}

static int adev_get_master_volume(struct p_audio_hw_device* dev, float* volume) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_master_volume(adev, volume);
}

static int adev_set_mode(struct p_audio_hw_device* dev, audio_mode_t mode) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_mode(adev, mode);
}

static int adev_set_mic_mute(struct p_audio_hw_device* dev, bool state) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_mic_mute(adev, state);
}

static int adev_get_mic_mute(const struct p_audio_hw_device* dev, bool* state) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_mic_mute(adev, state);
}

static int adev_set_parameters(struct p_audio_hw_device* dev, const char* kv_pairs) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_parameters(adev, kv_pairs);
}

static char* adev_get_parameters(const struct p_audio_hw_device* dev, const char* keys) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_parameters(adev, keys);
}

static size_t adev_get_input_buffer_size(const struct p_audio_hw_device* dev,
                                         const struct audio_config* config) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_input_buffer_size(adev, config);
}

static int adev_open_output_stream(struct p_audio_hw_device* dev, audio_io_handle_t handle,
                                   audio_devices_t devices, audio_output_flags_t flags,
                                   struct audio_config* config,
                                   struct audio_stream_out** stream_out, const char* address) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->open_output_stream(adev, handle, devices, flags, config, stream_out, address);
}

static void adev_close_output_stream(struct p_audio_hw_device* dev,
                                     struct audio_stream_out* stream_out) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    adev->close_output_stream(adev, stream_out);
}

static int adev_open_input_stream(struct p_audio_hw_device* dev, audio_io_handle_t handle,
                                  audio_devices_t devices, struct audio_config* config,
                                  struct p_audio_stream_in** stream_in, audio_input_flags_t flags,
                                  const char* address, audio_source_t source) {
    struct p_audio_hw_device* adev = to_amazon(dev);
    audio_source_t amazon_source = remap_input_source(source);
    struct p_audio_stream_in* amazon_stream = nullptr;
    struct p_audio_stream_in_ext* in;
    int rc;

    rc = adev->open_input_stream(adev, handle, devices, config, &amazon_stream, flags, address,
                                 amazon_source);
    if (rc)
        return rc;

    in = create_p_audio_stream_in_ext(amazon_stream);
    if (!in) {
        adev->close_input_stream(adev, amazon_stream);
        return -ENOMEM;
    }

    if (amazon_source == AUDIO_SOURCE_VOICE_COMMUNICATION)
        asp_send_command(ASP_CMD_SET_VOLUME, ASP_VOLUME_MAX);
    in->mic_capture = amazon_source != source;

    *stream_in = &in->stream;
    return 0;
}

static void adev_close_input_stream(struct p_audio_hw_device* dev,
                                    struct p_audio_stream_in* stream_in) {
    struct p_audio_hw_device* adev = to_amazon(dev);
    struct p_audio_stream_in_ext* in = reinterpret_cast<struct p_audio_stream_in_ext*>(stream_in);

    adev->close_input_stream(adev, in->amazon_stream);
    free(in);
}

static int adev_get_microphones(const struct p_audio_hw_device* dev,
                                struct audio_microphone_characteristic_t* mic_array,
                                size_t* mic_count) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_microphones(adev, mic_array, mic_count);
}

static int adev_dump(const struct p_audio_hw_device* dev, int fd) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->dump(adev, fd);
}

static int adev_set_master_mute(struct p_audio_hw_device* dev, bool mute) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_master_mute(adev, mute);
}

static int adev_get_master_mute(struct p_audio_hw_device* dev, bool* mute) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_master_mute(adev, mute);
}

static int adev_create_audio_patch(struct p_audio_hw_device* dev, unsigned int num_sources,
                                   const struct amazon_audio_port_config* sources,
                                   unsigned int num_sinks,
                                   const struct amazon_audio_port_config* sinks,
                                   audio_patch_handle_t* handle) {
    struct p_audio_hw_device* adev = to_amazon(dev);
    struct amazon_audio_port_config amazon_sinks[AUDIO_PATCH_PORTS_MAX];

    if (num_sinks > AUDIO_PATCH_PORTS_MAX)
        return -EINVAL;

    for (unsigned int i = 0; i < num_sinks; i++) {
        amazon_sinks[i] = sinks[i];
        if (amazon_sinks[i].type == AUDIO_PORT_TYPE_MIX)
            amazon_sinks[i].ext.mix.usecase.source =
                    remap_input_source(amazon_sinks[i].ext.mix.usecase.source);
    }

    return adev->create_audio_patch(adev, num_sources, sources, num_sinks, amazon_sinks, handle);
}

static int adev_release_audio_patch(struct p_audio_hw_device* dev, audio_patch_handle_t handle) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->release_audio_patch(adev, handle);
}

static int adev_get_audio_port(struct p_audio_hw_device* dev, struct audio_port* port) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->get_audio_port(adev, port);
}

static int adev_set_audio_port_config(struct p_audio_hw_device* dev,
                                      const struct amazon_audio_port_config* config) {
    struct p_audio_hw_device* adev = to_amazon(dev);

    return adev->set_audio_port_config(adev, config);
}

static int adev_close(hw_device_t* device) {
    struct p_wrapper_audio_device* ctx = reinterpret_cast<struct p_wrapper_audio_device*>(device);

    ctx->amazon_device->common.close(&ctx->amazon_device->common);
    free(ctx);

    return 0;
}

#define FORWARD(field) adev->device.field = adev->amazon_device->field ? adev_##field : nullptr

static int adev_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    struct p_wrapper_audio_device* adev;
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

    adev = (struct p_wrapper_audio_device*)calloc(1, sizeof(struct p_wrapper_audio_device));
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
    FORWARD(get_audio_port);
    FORWARD(set_audio_port_config);

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
