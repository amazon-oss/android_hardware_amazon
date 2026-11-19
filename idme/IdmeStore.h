/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fireos/hardware/idme/1.1/types.h>

#include <string>
#include <vector>

namespace fireos::hardware::idme::V1_1::implementation {

IdmeStatus ReadIdmeStore(const std::string& path, const std::string& name,
                         std::vector<uint8_t>* value);

}  // namespace fireos::hardware::idme::V1_1::implementation
