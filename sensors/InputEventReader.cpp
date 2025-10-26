/*
 * SPDX-FileCopyrightText: 2008 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/cdefs.h>
#include <sys/types.h>

#include <linux/input.h>

#include "InputEventReader.h"
#include "nusensors.h"

/*****************************************************************************/

struct input_event;

InputEventCircularReader::InputEventCircularReader(size_t numEvents)
    : mBuffer(new input_event[numEvents * 2]),

      mBufferEnd(mBuffer + numEvents),
      mHead(mBuffer),
      mCurr(mBuffer),
      mFreeSpace(numEvents) {}

InputEventCircularReader::~InputEventCircularReader() {
    delete[] mBuffer;
}

ssize_t InputEventCircularReader::fill(int fd) {
    size_t numEventsRead = 0;
    if (mFreeSpace) {
        const ssize_t nread = read(fd, mHead, mFreeSpace * sizeof(input_event));
        if (nread < 0 || nread % sizeof(input_event)) {
            // we got a partial event!!
            return nread < 0 ? -errno : -EINVAL;
        }

        numEventsRead = nread / sizeof(input_event);
        if (numEventsRead) {
            mHead += numEventsRead;
            mFreeSpace -= numEventsRead;
            if (mHead > mBufferEnd) {
                size_t s = mHead - mBufferEnd;
                memcpy(mBuffer, mBufferEnd, s * sizeof(input_event));
                mHead = mBuffer + s;
            }
        }
    }

    return numEventsRead;
}

ssize_t InputEventCircularReader::readEvent(input_event const** events) {
    *events = mCurr;
    ssize_t available = (mBufferEnd - mBuffer) - mFreeSpace;
    return available ? 1 : 0;
}

void InputEventCircularReader::next() {
    mCurr++;
    mFreeSpace++;
    if (mCurr >= mBufferEnd) {
        mCurr = mBuffer;
    }
}
