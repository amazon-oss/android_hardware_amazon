/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <binder/MemoryHeapBase.h>

#include <new>

extern "C" void _ZN7android14MemoryHeapBaseC1Eijjj(android::MemoryHeapBase* thiz, int fd,
                                                   size_t size, uint32_t flags, uint32_t offset) {
    new (thiz) android::MemoryHeapBase(fd, size, flags, static_cast<off_t>(offset));
}
