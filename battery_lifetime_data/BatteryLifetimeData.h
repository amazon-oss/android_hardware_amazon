/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <sys/types.h>

#include <array>
#include <cstdint>

#include <android-base/unique_fd.h>

class BatteryLifetimeData {
  public:
    explicit BatteryLifetimeData(android::base::unique_fd fd);

    void Restore();
    void Sync();

  private:
    enum Field {
        kCycle,
        kAgingFactor,
        kMaxVoltage,
        kMinVoltage,
        kMaxCurrent,
        kMinCurrent,
        kMaxTemperature,
        kMinTemperature,
        kFieldCount,
    };

    struct Record {
        std::array<int32_t, kFieldCount> data;
        uint32_t checksum;

        uint32_t Checksum() const;
        bool IsValid() const { return checksum == Checksum(); }
    };

    static_assert(sizeof(Record) == 36);

    bool Read(off64_t offset, Record* record) const;
    void Write();
    bool Update();

    android::base::unique_fd fd_;
    Record record_{};
};
