/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <unistd.h>

#include <utility>

#include <android-base/logging.h>
#include <android-base/unique_fd.h>

#include "BatteryLifetimeData.h"

using android::base::unique_fd;

constexpr char kProinfo[] = "/dev/block/platform/soc/11230000.mmc/by-name/proinfo";
constexpr unsigned int kSyncInterval = 1800;

int main() {
    unique_fd fd(open(kProinfo, O_RDWR | O_CLOEXEC));
    if (fd < 0) {
        PLOG(ERROR) << "Failed to open " << kProinfo;
        return 1;
    }

    BatteryLifetimeData data(std::move(fd));
    data.Restore();

    while (true) {
        sleep(kSyncInterval);
        data.Sync();
    }
}
