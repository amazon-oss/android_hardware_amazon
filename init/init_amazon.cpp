/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "log.h"
#include "vendor_init.h"

namespace {

const std::string kByName = "/dev/block/platform/mtk-msdc.0/by-name/";

struct AmonetLink {
    const char* partition;
    const char* alias;
    bool hide;
    bool optional;
};

constexpr AmonetLink kMt8127Links[] = {
        {"UBOOT", "UBOOT_real", true, false},
        {"TEE1", "TEE1_real", true, false},
        {"TEE2", "TEE2_real", true, false},
};

constexpr AmonetLink kMt8163Links[] = {
        {"boot", "boot_amonet", false, false}, {"recovery", "recovery_amonet", false, false},
        {"boot_x", "boot", false, false},      {"recovery_x", "recovery", false, false},
        {"lk", "lk_real", true, false},        {"lk2", "lk2_real", true, true},
        {"tee1", "tee1_real", true, false},    {"tee2", "tee2_real", true, false},
        {"boot0hdr0", "boot0hdr0_real", true, true}, {"boot0hdr1", "boot0hdr1_real", true, true},
        {"boot0img0", "boot0img0_real", true, true}, {"boot0img1", "boot0img1_real", true, true},
};

constexpr AmonetLink kMt8173Links[] = {
        {"boot", "boot_amonet", false, false}, {"recovery", "recovery_amonet", false, false},
        {"boot_x", "boot", false, false},      {"recovery_x", "recovery", false, false},
        {"lk", "lk_real", true, false},        {"TEE1", "TEE1_real", true, false},
        {"TEE2", "TEE2_real", true, false},
};

void Remove(const std::string& path) {
    if (unlink(path.c_str()) && errno != ENOENT)
        ERROR("Failed to remove %s: %s\n", path.c_str(), strerror(errno));
}

void Link(const std::string& target, const std::string& path) {
    Remove(path);
    if (symlink(target.c_str(), path.c_str()))
        ERROR("Failed to link %s to %s: %s\n", path.c_str(), target.c_str(), strerror(errno));
}

template <size_t N>
void SetupAmonetLinks(const AmonetLink (&links)[N]) {
    if (!access((kByName + links[0].alias).c_str(), F_OK)) return;

    std::vector<std::string> targets;
    for (const auto& link : links) {
        char target[PATH_MAX];
        if (!realpath((kByName + link.partition).c_str(), target)) {
            if (!link.optional) return;
            target[0] = '\0';
        }
        targets.emplace_back(target);
    }

    for (size_t i = 0; i < N; i++) {
        if (targets[i].empty()) continue;

        const std::string partition = kByName + links[i].partition;

        Remove(partition);
        Link(targets[i], kByName + links[i].alias);
        if (links[i].hide) Link("/dev/null", partition);
    }

    NOTICE("Set up amonet links\n");
}

}  // namespace

void vendor_load_properties() {
    SetupAmonetLinks(kMt8127Links);
    SetupAmonetLinks(kMt8163Links);
    SetupAmonetLinks(kMt8173Links);
}
