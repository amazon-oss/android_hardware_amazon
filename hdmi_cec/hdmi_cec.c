/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "hdmi_cec.amazon_wrapper"

#include <errno.h>

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/hdmi_cec.h>

static void (*amazon_set_option)(const struct hdmi_cec_device* dev, int flag, int value);

static void set_option(const struct hdmi_cec_device* dev, int flag, int value) {
    if (flag == HDMI_OPTION_SYSTEM_CEC_CONTROL && value) {
        amazon_set_option(dev, HDMI_OPTION_ENABLE_CEC, 1);
    }

    amazon_set_option(dev, flag, value);
}

static int open_cec(const struct hw_module_t* module __unused, const char* name,
                    struct hw_device_t** device) {
    const struct hw_module_t* amazon_module;
    hdmi_cec_device_t* dev;
    int rc;

    rc = hw_get_module_by_class(HDMI_CEC_HARDWARE_MODULE_ID, "amazon", &amazon_module);
    if (rc < 0) {
        ALOGE("Could not get HDMI-CEC module: %d", rc);
        return rc;
    }

    rc = amazon_module->methods->open(amazon_module, name, device);
    if (rc < 0) {
        ALOGE("Could not open HDMI-CEC device: %d", rc);
        return rc;
    }

    dev = (hdmi_cec_device_t*)*device;
    amazon_set_option = dev->set_option;
    dev->set_option = set_option;

    return 0;
}

static struct hw_module_methods_t hdmi_cec_module_methods = {
    .open = open_cec,
};

hdmi_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = HDMI_CEC_MODULE_API_VERSION_1_0,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = HDMI_CEC_HARDWARE_MODULE_ID,
        .name = "Amazon HDMI-CEC HAL wrapper",
        .author = "The LineageOS Project",
        .methods = &hdmi_cec_module_methods,
    },
};
