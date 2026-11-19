/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "IdmeProc.h"

#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace fireos::hardware::idme::V1_1::implementation {

namespace {

constexpr size_t kIdmeProcValueSize = 256;

}  // namespace

std::string ReadIdmeProc(const std::string& path) {
    android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), O_RDONLY | O_CLOEXEC)));
    if (fd < 0) {
        if (errno != ENOENT) {
            PLOG(ERROR) << "Failed to open " << path;
        }
        return {};
    }

    char buf[kIdmeProcValueSize];
    ssize_t len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)));
    if (len <= 0 || buf[0] == '\0' || buf[0] == '\n') {
        return {};
    }

    buf[len - 1] = '\0';
    return buf;
}

}  // namespace fireos::hardware::idme::V1_1::implementation
