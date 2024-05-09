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

#include <cutils/list.h>

#include "audio_amazon.h"

struct amazon_wrapper_audio_device {
    struct audio_hw_device device;
    struct amazon_audio_hw_device* amazon_device;
    pthread_mutex_t lock;
    audio_patch_handle_t next_patch_handle;
    struct listnode audio_patch_record_list;
    int hw_module;
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

static struct audio_patch_record* get_patch_from_list(struct audio_device* dev,
                                                      audio_patch_handle_t patch_id) {
    struct audio_patch_record* patch;
    struct listnode* node;
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);

    list_for_each(node, &ctx->audio_patch_record_list) {
        patch = node_to_item(node, struct audio_patch_record, list);
        if (patch->handle == patch_id) return patch;
    }
    return NULL;
}

const char* strAudioPatchRole[] = {"AUDIO_PORT_ROLE_NONE", "AUDIO_PORT_ROLE_SOURCE",
                                   "AUDIO_PORT_ROLE_SINK"};
const char* strAudioPatchType[] = {"AUDIO_PORT_TYPE_NONE", "AUDIO_PORT_TYPE_DEVICE",
                                   "AUDIO_PORT_TYPE_MIX", "AUDIO_PORT_TYPE_SESSION"};

static int adev_create_audio_patch(struct audio_hw_device* dev, unsigned int num_sources,
                                   const struct audio_port_config* sources, unsigned int num_sinks,
                                   const struct audio_port_config* sinks,
                                   audio_patch_handle_t* handle) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);
    int status = 0;
    struct audio_patch_record* patch_record = nullptr;
    audio_patch_handle_t handle_wrapper = AUDIO_PATCH_HANDLE_NONE;
    audio_patch_handle_t handle_amazon = AUDIO_PATCH_HANDLE_NONE;
    if (!ctx) {
        return status;
    }
    if (num_sources != 1 || num_sinks == 0 || num_sinks > AUDIO_PATCH_PORTS_MAX) {
        return -EINVAL;
    }
    if (sources[0].type == AUDIO_PORT_TYPE_DEVICE) {
        if (num_sinks != 1 || sinks[0].type != AUDIO_PORT_TYPE_MIX) {
            return -EINVAL;
        }
    } else if (sources[0].type == AUDIO_PORT_TYPE_MIX) {
        for (unsigned int i = 0; i < num_sinks; i++) {
            if (sinks[i].type != AUDIO_PORT_TYPE_DEVICE) {
                ALOGE("%s() invalid sink type %#x for mix source", __func__, sinks[i].type);
                return -EINVAL;
            }
        }
    } else {
        return -EINVAL;
    }
    if (*handle == AUDIO_PATCH_HANDLE_NONE) {
        patch_record = (struct audio_patch_record*)calloc(1, sizeof(struct audio_patch_record));
        if (!patch_record) {
            ALOGE("%s() failed to allocate memory for patch record", __func__);
            status = -ENOMEM;
            goto error;
        }
        pthread_mutex_lock(&ctx->lock);
        handle_wrapper = ++ctx->next_patch_handle;
        patch_record->handle = handle_wrapper;
        patch_record->handle_amazon = AUDIO_PATCH_HANDLE_NONE;
        patch_record->patch.id = handle_wrapper;
        patch_record->usecase = USECASE_INVALID;
        patch_record->patch.num_sources = num_sources;
        patch_record->patch.num_sinks = num_sinks;
        for (int i = 0; i < num_sources; i++) patch_record->patch.sources[i] = sources[i];
        for (int i = 0; i < num_sinks; i++) patch_record->patch.sinks[i] = sinks[i];

        for (int i = 0; i < num_sources; i++) {
            if (patch_record->patch.sources[i].type == AUDIO_PORT_TYPE_MIX) {
                patch_record->patch.sources[i].ext.mix.hw_module =
                        patch_record->patch.sources[i].ext.mix.handle;
                ALOGD("%s - change source hw_module with handle(%d) ", __FUNCTION__,
                      patch_record->patch.sources[i].ext.mix.hw_module);
            }
            if (patch_record->patch.sources[i].type == AUDIO_PORT_TYPE_DEVICE) {
                patch_record->patch.sources[i].ext.device.hw_module =
                        patch_record->patch.sources[i].ext.device.type;
            }
        }

        for (int i = 0; i < num_sinks; i++) {
            if (patch_record->patch.sinks[i].type == AUDIO_PORT_TYPE_MIX) {
                patch_record->patch.sinks[i].ext.mix.hw_module =
                        patch_record->patch.sinks[i].ext.mix.handle;
                ALOGD("%s - change sink hw_module with handle(%d) ", __FUNCTION__,
                      patch_record->patch.sinks[i].ext.mix.hw_module);
            }

            if (patch_record->patch.sinks[i].type == AUDIO_PORT_TYPE_DEVICE) {
                patch_record->patch.sinks[i].ext.device.hw_module =
                        patch_record->patch.sinks[i].ext.device.type;
            }
        }

        status = ctx->amazon_device->create_audio_patch(
                ctx->amazon_device, patch_record->patch.num_sources, patch_record->patch.sources,
                patch_record->patch.num_sinks, patch_record->patch.sinks, &handle_amazon);

        if (status == 0) {
            ALOGD("%s - call create_audio_patch sucess", __FUNCTION__);
            patch_record->handle_amazon = handle_amazon;
            ctx->hw_module = 0;
        } else {
            patch_record->handle_amazon = AUDIO_PATCH_HANDLE_NONE;
            ALOGD("%s - call create_audio_patch error %i", __FUNCTION__, status);
        }

        list_add_tail(&ctx->audio_patch_record_list, &patch_record->list);
        pthread_mutex_unlock(&ctx->lock);
        *handle = handle_wrapper;
        status = 0;
    }
    goto exit;
error:
    free(patch_record);
exit:
    return status;
}

static int adev_release_audio_patch(struct audio_hw_device* dev, audio_patch_handle_t handle) {
    amazon_wrapper_audio_device* ctx = reinterpret_cast<amazon_wrapper_audio_device*>(dev);
    struct audio_device* adev = (struct audio_device*)dev;
    struct audio_patch_record* patch_record = NULL;
    int status = 0;

    if (!ctx) {
        return status;
    }

    if (handle == AUDIO_PATCH_HANDLE_NONE) {
        ALOGW("%s() empty handle passed", __func__);
        status = -EINVAL;
        return status;
    }

    pthread_mutex_lock(&ctx->lock);
    patch_record = get_patch_from_list(adev, handle);
    pthread_mutex_unlock(&ctx->lock);

    if (!patch_record) {
        ALOGE("%s() failed to find the patch record with handle (%d) in the list", __func__,
              handle);
        status = -EINVAL;
        return status;
    } else {
        status = ctx->amazon_device->release_audio_patch(ctx->amazon_device, patch_record->handle_amazon);
    }

    pthread_mutex_lock(&ctx->lock);
    list_remove(&patch_record->list);
    pthread_mutex_unlock(&ctx->lock);
    free(patch_record);

    return status;
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

    pthread_mutex_init(&adev->lock, (const pthread_mutexattr_t*)NULL);
    list_init(&adev->audio_patch_record_list);
    adev->next_patch_handle = AUDIO_PATCH_HANDLE_NONE;
    adev->hw_module = 1;

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
    adev->device.create_audio_patch = adev_create_audio_patch;
    adev->device.release_audio_patch = adev_release_audio_patch;

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
        .author = "Sebastiano Barezzi, Raphael Mounier and Roger Ortiz, The LineageOS Project",
        .methods = &hal_module_methods,
    },
};
// clang-format on
