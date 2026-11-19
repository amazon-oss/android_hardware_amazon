/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Idme.h"

#include "IdmeProc.h"
#include "IdmeStore.h"

namespace fireos::hardware::idme::V1_1::implementation {

using ::android::hardware::Void;

namespace {

constexpr char kIdmeProcPath[] = "/proc/idme/";
constexpr char kIdmeStorePath[] = "/dev/block/mmcblk0boot1";

std::string ReadProcValue(const std::string& name) {
    return ReadIdmeProc(kIdmeProcPath + name);
}

}  // namespace

Return<void> Idme::getProductName(getProductName_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("product_name"));
    return Void();
}

Return<void> Idme::getMacAddress(getMacAddress_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("mac_addr"));
    return Void();
}

Return<void> Idme::getSecret(getSecret_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("mac_sec"));
    return Void();
}

Return<void> Idme::getSerialNumber(getSerialNumber_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("serial"));
    return Void();
}

Return<void> Idme::getDeviceTypeId(getDeviceTypeId_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("device_type_id"));
    return Void();
}

Return<void> Idme::getBtMacAddress(getBtMacAddress_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("bt_mac_addr"));
    return Void();
}

Return<void> Idme::getBtMfg(getBtMfg_cb _hidl_cb) {
    _hidl_cb(ReadProcValue("bt_mfg"));
    return Void();
}

Return<void> Idme::getUnlockCode(getUnlockCode_cb _hidl_cb) {
    std::vector<uint8_t> value;
    IdmeStatus status = ReadIdmeStore(kIdmeStorePath, "unlock_code", &value);
    _hidl_cb(status, value);
    return Void();
}

Return<void> Idme::getUnlockVersion(getUnlockVersion_cb _hidl_cb) {
    std::vector<uint8_t> value;
    IdmeStatus status = ReadIdmeStore(kIdmeStorePath, "unlock_version", &value);
    _hidl_cb(status, value);
    return Void();
}

}  // namespace fireos::hardware::idme::V1_1::implementation
