/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>
#include <vector>

class ThermalPolicy {
  public:
    bool Load(const std::string& path);
    void Apply() const;

  private:
    struct Zone {
        std::string name;
        std::string governor;
        std::string polling;
        std::vector<std::string> cdevs;
        std::vector<std::string> trips;
        std::vector<std::string> hysts;
        std::map<std::string, std::vector<std::string>> levels;
    };

    void ApplyZone(const Zone& zone) const;

    std::vector<Zone> zones_;
};
