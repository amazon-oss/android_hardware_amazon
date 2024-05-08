/*
 * Copyright (C) 2024 The LineageOS Project
 *
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

#include "audio_amazon.h"

struct amazon_wrapper_audio_device {
    struct audio_hw_device device;
    struct amazon_audio_hw_device* amazon_device;
};

static int adev_open_output_stream(struct audio_hw_device* dev, audio_io_handle_t handle,
                                   audio_devices_t devices, audio_output_flags_t flags,
                                   struct audio_config* config,
                                   struct audio_stream_out** stream_out, const char* address) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->open_output_stream(ctx->amazon_device, handle, devices, flags, config,
                                                stream_out, address);
}

static void adev_close_output_stream(struct audio_hw_device* dev, struct audio_stream_out* stream) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->close_output_stream(ctx->amazon_device, stream);
}

static int adev_set_parameters(struct audio_hw_device* dev, const char* kvpairs) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->set_parameters(ctx->amazon_device, kvpairs);
}

static char* adev_get_parameters(const struct audio_hw_device* dev, const char* keys) {
    const amazon_wrapper_audio_device* ctx = reinterpret_cast<const amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->get_parameters(ctx->amazon_device, keys);
}

static int adev_init_check(const struct audio_hw_device* dev) {
    const amazon_wrapper_audio_device* ctx = reinterpret_cast<const amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->init_check(ctx->amazon_device);
}

static int adev_set_voice_volume(struct audio_hw_device* dev, float volume) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->set_voice_volume(ctx->amazon_device, volume);
}

static int adev_set_master_volume(struct audio_hw_device* dev, float volume) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->set_master_volume(ctx->amazon_device, volume);
}

static int adev_get_master_volume(struct audio_hw_device* dev, float* volume) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->get_master_volume(ctx->amazon_device, volume);
}

static int adev_set_master_mute(struct audio_hw_device* dev, bool muted) {
    ALOGW("%s(): not implemented yet!", __func__);
    return 0;
}

static int adev_get_master_mute(struct audio_hw_device* dev __unused, bool* muted __unused) {
    ALOGW("%s(): not implemented yet!", __func__);
    return 0;
}

static int adev_set_mode(struct audio_hw_device* dev, audio_mode_t mode) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->set_mode(ctx->amazon_device, mode);
}

static int adev_set_mic_mute(struct audio_hw_device* dev, bool state) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->set_mic_mute(ctx->amazon_device, state);
}

static int adev_get_mic_mute(const struct audio_hw_device* dev, bool* state) {
    const amazon_wrapper_audio_device* ctx = reinterpret_cast<const amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->get_mic_mute(ctx->amazon_device, state);
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device* dev,
                                         const struct audio_config* config) {
    const amazon_wrapper_audio_device* ctx = reinterpret_cast<const amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->get_input_buffer_size(ctx->amazon_device, config);
}

static int adev_open_input_stream(struct audio_hw_device* dev, audio_io_handle_t handle,
                                  audio_devices_t devices, struct audio_config* config,
                                  struct audio_stream_in** stream_in, audio_input_flags_t flags,
                                  const char* address, audio_source_t source) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->open_input_stream(ctx->amazon_device, handle, devices, config, stream_in,
                                               flags, address, source);
}

static void adev_close_input_stream(struct audio_hw_device* dev, struct audio_stream_in* in) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->close_input_stream(ctx->amazon_device, in);
}

static int adev_dump(const struct audio_hw_device* dev, int fd) {
    const amazon_wrapper_audio_device* ctx = reinterpret_cast<const amazon_wrapper_audio_device*>(dev);

    return ctx->amazon_device->dump(ctx->amazon_device, fd);
}

static int adev_close(hw_device_t* device) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(device);

    if (!ctx) {
        return 0;
    }

    // Close the Amazon audio device
    ctx->amazon_device->common.close((struct hw_device_t*)ctx->amazon_device);

    free(ctx);

    return 0;
}

static int adev_open(const hw_module_t* module, const char* name, hw_device_t** device) {
    int rc;
    struct amazon_wrapper_audio_device* adev;
    struct hw_module_t* amazon_module;

    ALOGI("adev_open: %s", name);

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0) {
        return -EINVAL;
    }

    adev = (struct amazon_wrapper_audio_device*)calloc(1, sizeof(struct amazon_wrapper_audio_device));
    if (!adev) {
        return -ENOMEM;
    }

    // Get the Amazon audio module
    rc = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID, "primary_amazon",
                                (const hw_module_t**)&amazon_module);
    if (rc < 0) {
        ALOGE("Could not get audio hw module");
        free(adev);
        return rc;
    }

    // Open it
    rc = amazon_module->methods->open(amazon_module, name, (struct hw_device_t**)&adev->amazon_device);
    if (rc < 0) {
        ALOGE("Could not open amazon audio hw device");
        free(adev);
        return rc;
    }

    adev->device.common.tag = HARDWARE_DEVICE_TAG;
    adev->device.common.version = HARDWARE_DEVICE_API_VERSION(3, 0);
    adev->device.common.module = (struct hw_module_t*)module;
    adev->device.common.close = adev_close;

    adev->device.init_check = adev_init_check;
    adev->device.set_voice_volume = adev_set_voice_volume;
    adev->device.set_master_volume = adev_set_master_volume;
    adev->device.get_master_volume = adev_get_master_volume;
    adev->device.set_master_mute = adev_set_master_mute;
    adev->device.get_master_mute = adev_get_master_mute;
    adev->device.set_mode = adev_set_mode;
    adev->device.set_mic_mute = adev_set_mic_mute;
    adev->device.get_mic_mute = adev_get_mic_mute;
    adev->device.set_parameters = adev_set_parameters;
    adev->device.get_parameters = adev_get_parameters;
    adev->device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->device.open_output_stream = adev_open_output_stream;
    adev->device.close_output_stream = adev_close_output_stream;
    adev->device.open_input_stream = adev_open_input_stream;
    adev->device.close_input_stream = adev_close_input_stream;
    adev->device.dump = adev_dump;

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
        .author = "Sebastiano Barezzi, Roger Ortiz, The LineageOS Project",
        .methods = &hal_module_methods,
    },
};
// clang-format on
