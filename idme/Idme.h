/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fireos/hardware/idme/1.1/IIdme.h>

namespace fireos::hardware::idme::V1_1::implementation {

using ::android::hardware::Return;

class Idme : public IIdme {
  public:
    Return<void> getProductName(getProductName_cb _hidl_cb) override;
    Return<void> getMacAddress(getMacAddress_cb _hidl_cb) override;
    Return<void> getSecret(getSecret_cb _hidl_cb) override;
    Return<void> getSerialNumber(getSerialNumber_cb _hidl_cb) override;
    Return<void> getDeviceTypeId(getDeviceTypeId_cb _hidl_cb) override;
    Return<void> getBtMacAddress(getBtMacAddress_cb _hidl_cb) override;
    Return<void> getBtMfg(getBtMfg_cb _hidl_cb) override;

    Return<void> getUnlockCode(getUnlockCode_cb _hidl_cb) override;
    Return<void> getUnlockVersion(getUnlockVersion_cb _hidl_cb) override;
};

}  // namespace fireos::hardware::idme::V1_1::implementation
