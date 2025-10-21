/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "sensors.echo"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <hardware/sensors.h>
#include <math.h>

#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <math.h>

#include <log/log.h>

#include "LightSensor.h"
#include "ProximitySensor.h"
#include "nusensors.h"

/*****************************************************************************/

struct sensors_poll_context_t {
    sensors_poll_device_1_t device;  // must be first

    sensors_poll_context_t();
    ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);
    int batch(int handle, int flags, int64_t period_ns, int64_t timeout);
    int flush(int handle);
    bool getInitialized() { return mInitialized; };

  private:
    bool mInitialized;

    enum {
        light = 0,
        proximity,
        numSensorDrivers,
        numFds,
    };

    static const size_t flushPipe = numFds - 1;
    struct pollfd mPollFds[numFds];
    int mFlushWritePipeFd;
    SensorBase* mSensors[numSensorDrivers];

    int handleToDriver(int handle) const {
        switch (handle) {
            case ID_LIGHT:
                return light;
            case ID_PROXIMITY:
                return proximity;
        }
        return -EINVAL;
    }
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t() {
    mInitialized = false;
    /* Must clean this up early or else the destructor will make a mess */
    memset(mSensors, 0, sizeof(mSensors));

    mSensors[light] = new LightSensor();
    mPollFds[light].fd = mSensors[light]->getFd();
    mPollFds[light].events = POLLIN;
    mPollFds[light].revents = 0;

    mSensors[proximity] = new ProximitySensor();
    mPollFds[proximity].fd = mSensors[proximity]->getFd();
    mPollFds[proximity].events = POLLIN;
    mPollFds[proximity].revents = 0;

    int flushFds[2];
    int result = pipe(flushFds);
    if (result < 0) ALOGE("error creating flush pipe (%s)", strerror(errno));
    result = fcntl(flushFds[0], F_SETFL, O_NONBLOCK);
    if (result < 0) ALOGE("error setting flushFds[0] access mode (%s)", strerror(errno));
    result = fcntl(flushFds[1], F_SETFL, O_NONBLOCK);
    if (result < 0) ALOGE("error setting flushFds[1] access mode (%s)", strerror(errno));
    mFlushWritePipeFd = flushFds[1];

    mPollFds[flushPipe].fd = flushFds[0];
    mPollFds[flushPipe].events = POLLIN;
    mPollFds[flushPipe].revents = 0;

    mInitialized = true;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i = 0; i < numSensorDrivers; i++) {
        delete mSensors[i];
    }
    close(mPollFds[flushPipe].fd);
    close(mFlushWritePipeFd);
    mInitialized = false;
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    if (!mInitialized) return -EINVAL;
    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->enable(handle, enabled);
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {
    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->setDelay(handle, ns);
}

int sensors_poll_context_t::batch(int handle, int flags, int64_t period_ns, int64_t timeout) {
    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->batch(handle, flags, period_ns, timeout);
}

int sensors_poll_context_t::flush(int handle) {
    int result;
    sensors_event_t flush_event_data;

    int index = handleToDriver(handle);
    if (index < 0) return index;

    result = mSensors[index]->isActivated(handle);
    if (!result) return -EINVAL;

    flush_event_data.sensor = 0;
    flush_event_data.timestamp = 0;
    flush_event_data.meta_data.sensor = handle;
    flush_event_data.meta_data.what = META_DATA_FLUSH_COMPLETE;
    flush_event_data.type = SENSOR_TYPE_META_DATA;
    flush_event_data.version = META_DATA_VERSION;

    result = write(mFlushWritePipeFd, &flush_event_data, sizeof(sensors_event_t));
    if (result < 0) ALOGE("error sending flush event data (%s)", strerror(errno));

    return (result >= 0 ? 0 : result);
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count) {
    int nbEvents = 0;
    int nb, polltime = -1;

    // look for new events
    nb = poll(mPollFds, numFds, polltime);

    /* flush event data */
    if ((count > 0) && (nb > 0)) {
        if (mPollFds[flushPipe].revents & POLLIN) {
            int n = read(mPollFds[flushPipe].fd, data, count * sizeof(sensors_event_t));
            if (n < 0) {
                ALOGE("error reading from flush pipe (%s)", strerror(errno));
                return 0;
            }
            nb = n / sizeof(sensors_event_t);
            mPollFds[flushPipe].revents = 0;
            count -= nb;
            nbEvents += nb;
            data += nb;
            return nbEvents;
        }
    }

    if (nb > 0) {
        for (int i = 0; count && i < numSensorDrivers; i++) {
            SensorBase* const sensor(mSensors[i]);
            if (mPollFds[i].revents & POLLIN) {
                nb = 0;
                nb = sensor->readEvents(data, count);
                mPollFds[i].revents = 0;

                if (nb > 0) {
                    count -= nb;
                    nbEvents += nb;
                    data += nb;
                }
            }
        }
    }

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t* dev) {
    sensors_poll_context_t* ctx = (sensors_poll_context_t*)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t* dev, int handle, int enabled) {
    sensors_poll_context_t* ctx = (sensors_poll_context_t*)dev;
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t* dev, int handle, int64_t ns) {
    sensors_poll_context_t* ctx = (sensors_poll_context_t*)dev;
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t* dev, sensors_event_t* data, int count) {
    sensors_poll_context_t* ctx = (sensors_poll_context_t*)dev;
    return ctx->pollEvents(data, count);
}

static int poll__batch(struct sensors_poll_device_1* dev, int handle, int flags, int64_t period_ns,
                       int64_t timeout) {
    sensors_poll_context_t* ctx = (sensors_poll_context_t*)dev;
    return ctx->batch(handle, flags, period_ns, timeout);
}

static int poll__flush(struct sensors_poll_device_1* dev, int handle) {
    sensors_poll_context_t* ctx = (sensors_poll_context_t*)dev;
    return ctx->flush(handle);
}

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device) {
    int status = -EINVAL;

    sensors_poll_context_t* dev = new sensors_poll_context_t();
    if (!dev->getInitialized()) {
        ALOGE("Failed to open the sensors");
        return status;
    }
    memset(&dev->device, 0, sizeof(sensors_poll_device_1));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = SENSORS_DEVICE_API_VERSION_1_3;
    dev->device.common.module = const_cast<hw_module_t*>(module);
    dev->device.common.close = poll__close;
    dev->device.activate = poll__activate;
    dev->device.setDelay = poll__setDelay;
    dev->device.poll = poll__poll;

    /* Batch processing */
    dev->device.batch = poll__batch;
    dev->device.flush = poll__flush;

    *device = &dev->device.common;

    status = 0;
    return status;
}
