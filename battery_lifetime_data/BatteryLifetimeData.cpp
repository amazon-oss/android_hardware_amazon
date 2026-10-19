/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BatteryLifetimeData.h"

#include <unistd.h>

#include <string>
#include <utility>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>

using android::base::ParseInt;
using android::base::ReadFileToString;
using android::base::ReadFullyAtOffset;
using android::base::Trim;
using android::base::unique_fd;
using android::base::WriteStringToFile;

namespace {

constexpr off64_t kRecordOffsets[] = {0x0, 0x3fdc};
constexpr int32_t kMaxCycleStep = 2;

const std::string kBatteryMeter = "/sys/devices/platform/battery_meter/";

constexpr const char* kNodes[] = {
        "FG_Battery_Cycle",           "FG_Aging_Factor",
        "FG_Max_Battery_Voltage",     "FG_Min_Battery_Voltage",
        "FG_Max_Battery_Current",     "FG_Min_Battery_Current",
        "FG_Max_Battery_Temperature", "FG_Min_Battery_Temperature",
};

bool ReadNode(int field, int32_t* value) {
    std::string buf;

    return ReadFileToString(kBatteryMeter + kNodes[field], &buf) && ParseInt(Trim(buf), value);
}

void WriteNode(int field, int32_t value) {
    if (!WriteStringToFile(std::to_string(value), kBatteryMeter + kNodes[field]))
        PLOG(ERROR) << "Failed to write " << kNodes[field];
}

}  // namespace

uint32_t BatteryLifetimeData::Record::Checksum() const {
    uint32_t sum = 0;

    for (size_t i = 0; i < data.size(); i++) {
        if (i % 2)
            sum += data[i];
        else
            sum ^= data[i];
    }

    return sum;
}

BatteryLifetimeData::BatteryLifetimeData(unique_fd fd) : fd_(std::move(fd)) {}

void BatteryLifetimeData::Restore() {
    Record primary, backup;
    bool primary_ok = Read(kRecordOffsets[0], &primary);
    bool backup_ok = Read(kRecordOffsets[1], &backup);

    if (primary_ok || backup_ok) {
        record_ = primary_ok ? primary : backup;

        for (int i = 0; i < kFieldCount; i++) {
            if (i != kAgingFactor) WriteNode(i, record_.data[i]);
        }

        LOG(INFO) << "Restored battery cycle " << record_.data[kCycle];
    } else {
        for (int i = 0; i < kFieldCount; i++) ReadNode(i, &record_.data[i]);

        LOG(INFO) << "No battery lifetime data, starting from the current values";
    }

    if (!primary_ok || !backup_ok) Write();
}

void BatteryLifetimeData::Sync() {
    if (Update()) Write();
}

bool BatteryLifetimeData::Read(off64_t offset, Record* record) const {
    return ReadFullyAtOffset(fd_, record, sizeof(*record), offset) && record->IsValid();
}

void BatteryLifetimeData::Write() {
    record_.checksum = record_.Checksum();

    for (off64_t offset : kRecordOffsets) {
        ssize_t ret = TEMP_FAILURE_RETRY(pwrite64(fd_, &record_, sizeof(record_), offset));
        if (ret != sizeof(record_)) PLOG(ERROR) << "Failed to write the record at " << offset;
    }

    fsync(fd_);
}

bool BatteryLifetimeData::Update() {
    auto& data = record_.data;
    bool changed = false;
    int32_t value;

    if (ReadNode(kCycle, &value)) {
        if (value < 0 && data[kCycle] != -value) {
            data[kCycle] = -value;
            changed = true;
        } else if (value > data[kCycle] && value - data[kCycle] <= kMaxCycleStep) {
            data[kCycle] = value;
            changed = true;
        }
    }

    if (ReadNode(kAgingFactor, &value) && value >= 0 && value <= 100 &&
        value != data[kAgingFactor]) {
        data[kAgingFactor] = value;
        changed = true;
    }

    for (int max = kMaxVoltage; max < kFieldCount; max += 2) {
        int min = max + 1;

        if (ReadNode(max, &value) && value > data[max]) {
            data[max] = value;
            changed = true;
        }

        if (ReadNode(min, &value) && value < data[min] && (min != kMinVoltage || value >= 0)) {
            data[min] = value;
            changed = true;
        }
    }

    return changed;
}
