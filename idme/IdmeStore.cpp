/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "IdmeStore.h"

#include <android-base/logging.h>
#include <android-base/unique_fd.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace fireos::hardware::idme::V1_1::implementation {

namespace {

constexpr char kIdmeMagic[] = "beefdeed";
constexpr size_t kIdmeSize = 0x6000;
constexpr size_t kIdmeAlignSize = 4;
constexpr uint32_t kIdmeMaxValueSize = 0x10000;

struct IdmeHeader {
    char magic[8];
    char version[4];
    int32_t itemsNum;
};

struct IdmeDesc {
    char name[16];
    uint32_t size;
    uint32_t exportable;
    uint32_t permission;
};

static_assert(sizeof(IdmeHeader) == 0x10);
static_assert(sizeof(IdmeDesc) == 0x1c);

size_t AlignSize(uint32_t size) {
    return (static_cast<size_t>(size) + kIdmeAlignSize - 1) & ~(kIdmeAlignSize - 1);
}

}  // namespace

IdmeStatus ReadIdmeStore(const std::string& path, const std::string& name,
                         std::vector<uint8_t>* value) {
    value->clear();

    android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), O_RDONLY | O_CLOEXEC)));
    if (fd < 0) {
        PLOG(ERROR) << "Failed to open " << path;
        return IdmeStatus::IDME_STATUS_ERROR;
    }

    std::vector<uint8_t> store(kIdmeSize);
    ssize_t len = TEMP_FAILURE_RETRY(read(fd, store.data(), store.size()));
    if (len < 0) {
        PLOG(ERROR) << "Failed to read " << path;
        return IdmeStatus::IDME_STATUS_ERROR;
    }

    if (len == 0) {
        return IdmeStatus::IDME_STATUS_NOT_FOUND;
    }

    if (static_cast<size_t>(len) != store.size()) {
        LOG(ERROR) << "Short read from " << path << ": " << len;
        return IdmeStatus::IDME_STATUS_ERROR;
    }

    IdmeHeader header;
    memcpy(&header, store.data(), sizeof(header));
    if (memcmp(header.magic, kIdmeMagic, sizeof(header.magic)) != 0) {
        LOG(ERROR) << "Invalid IDME magic in " << path;
        return IdmeStatus::IDME_STATUS_ERROR;
    }

    size_t offset = sizeof(header);
    for (int32_t i = 0; i < header.itemsNum && offset + sizeof(IdmeDesc) <= store.size(); i++) {
        IdmeDesc desc;
        memcpy(&desc, store.data() + offset, sizeof(desc));
        offset += sizeof(desc);

        if (strncmp(desc.name, name.c_str(), sizeof(desc.name)) != 0) {
            offset += AlignSize(desc.size);
            continue;
        }

        if (desc.size > kIdmeMaxValueSize || desc.size > store.size() - offset) {
            LOG(ERROR) << "Invalid size of " << name << ": " << desc.size;
            return IdmeStatus::IDME_STATUS_SIZE_ERROR;
        }

        value->assign(store.begin() + offset, store.begin() + offset + desc.size);
        return IdmeStatus::IDME_STATUS_OK;
    }

    return IdmeStatus::IDME_STATUS_NOT_FOUND;
}

}  // namespace fireos::hardware::idme::V1_1::implementation
