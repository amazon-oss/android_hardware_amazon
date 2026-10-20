/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ThermalPolicy.h"

#include <dirent.h>

#include <algorithm>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <utility>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>

using android::base::Join;
using android::base::ParseUint;
using android::base::ReadFileToString;
using android::base::StartsWith;
using android::base::Trim;
using android::base::WriteStringToFile;

namespace {

const std::string kThermalPath = "/sys/class/thermal/";
const std::string kZonePrefix = "thermal_zone";
const std::string kCdevPrefix = "cooling_device";

std::vector<std::string> Tokenize(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;

    while (stream >> token) tokens.push_back(token);

    return tokens;
}

std::optional<std::string> FindNode(const std::string& prefix, const std::string& type) {
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(kThermalPath.c_str()), closedir);
    if (!dir) return std::nullopt;

    std::vector<std::string> names;
    while (dirent* entry = readdir(dir.get())) {
        if (StartsWith(entry->d_name, prefix)) names.emplace_back(entry->d_name);
    }
    std::sort(names.begin(), names.end());

    for (const auto& name : names) {
        std::string value;
        if (ReadFileToString(kThermalPath + name + "/type", &value) && Trim(value) == type)
            return kThermalPath + name;
    }

    return std::nullopt;
}

void WriteNode(const std::string& path, const std::string& value) {
    if (!WriteStringToFile(value, path))
        PLOG(ERROR) << "Failed to write " << value << " to " << path;
}

}  // namespace

bool ThermalPolicy::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file) return false;

    Zone zone;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<std::string> values = Tokenize(line);
        if (values.empty()) continue;

        std::string key = values.front();
        values.erase(values.begin());

        if (key == "[") {
            zone = {};
        } else if (key == "]") {
            zones_.push_back(std::move(zone));
            zone = {};
        } else if (key == "zone") {
            zone.name = values.empty() ? "" : values.front();
        } else if (key == "gov") {
            zone.governor = values.empty() ? "" : values.front();
        } else if (key == "poll") {
            zone.polling = values.empty() ? "" : values.front();
        } else if (key == "cdevs") {
            zone.cdevs = std::move(values);
        } else if (key == "trip") {
            zone.trips = std::move(values);
        } else if (key == "hyst") {
            zone.hysts = std::move(values);
        } else if (std::find(zone.cdevs.begin(), zone.cdevs.end(), key) != zone.cdevs.end()) {
            zone.levels[key] = std::move(values);
        }
    }

    return true;
}

void ThermalPolicy::Apply() const {
    for (const auto& zone : zones_) ApplyZone(zone);
}

void ThermalPolicy::ApplyZone(const Zone& zone) const {
    std::optional<std::string> tz = FindNode(kZonePrefix, zone.name);
    if (!tz) return;

    std::vector<std::pair<std::string, std::string>> cdevs;
    for (const auto& cdev : zone.cdevs) {
        if (std::optional<std::string> path = FindNode(kCdevPrefix, cdev))
            cdevs.emplace_back(cdev, *path);
    }
    if (cdevs.empty()) return;

    std::string buf;
    unsigned int trips = 0;
    if (!ReadFileToString(*tz + "/trips", &buf) || !ParseUint(Trim(buf), &trips) ||
        trips != zone.trips.size()) {
        LOG(WARNING) << zone.name << ": expected " << trips << " trips, policy has "
                     << zone.trips.size();
        return;
    }

    if (!zone.governor.empty()) WriteNode(*tz + "/policy", zone.governor);

    for (size_t i = 0; i < zone.trips.size(); i++)
        WriteNode(*tz + "/trip_point_" + std::to_string(i) + "_temp", zone.trips[i]);

    for (size_t i = 0; i < zone.hysts.size(); i++)
        WriteNode(*tz + "/trip_point_" + std::to_string(i) + "_hyst", zone.hysts[i]);

    std::string id = tz->substr(kThermalPath.size() + kZonePrefix.size());
    std::vector<std::string> bound;

    for (const auto& [cdev, path] : cdevs) {
        bound.push_back(cdev);

        auto levels = zone.levels.find(cdev);
        if (levels == zone.levels.end()) continue;

        const std::vector<std::string>& states = levels->second;
        for (size_t trip = 0; trip < states.size(); trip++)
            WriteNode(path + "/levels", std::to_string(trip) + " " + states[trip] + " " + id);
    }

    if (!zone.polling.empty()) WriteNode(*tz + "/polling", zone.polling);

    LOG(INFO) << zone.name << ": bound " << Join(bound, ' ');
}
