/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "init_amazon.h"
#include "init_amonet.h"

void init_symlink_info(symlink_info_t *info, const char *src, const char *dst,
                       int redirect) {
    snprintf(info->src_name, sizeof(info->src_name), "%s%s", PATH_PREFIX, src);
    snprintf(info->dst_name, sizeof(info->dst_name), "%s%s", PATH_PREFIX, dst);
    info->redirect_to_null = redirect;
    LOG(ERROR) << "Initialized symlink info: src=" << info->src_name << ", dst=" << info->dst_name << ", redirect=" << redirect;
}

const char *resolve_symlink(const char *path) {
    static char resolved_path[PATH_MAX];
    if (realpath(path, resolved_path)) {
        LOG(ERROR) << "Resolved symlink: " << path << " -> " << resolved_path;
        return resolved_path;
    }
    LOG(ERROR) << "Failed to resolve symlink for path: " << path;
    return NULL;
}

void init_amonet_symlinks() {
    symlink_info_t symlinks[10];

    init_symlink_info(&symlinks[0], "boot", "boot_amonet", 0);
    init_symlink_info(&symlinks[1], "recovery", "recovery_amonet", 0);
    init_symlink_info(&symlinks[2], "boot_x", "boot", 0);
    init_symlink_info(&symlinks[3], "recovery_x", "recovery", 0);
    init_symlink_info(&symlinks[4], "UBOOT", "UBOOT_real", 1);
    init_symlink_info(&symlinks[5], "TEE1", "TEE1_real", 1);
    init_symlink_info(&symlinks[6], "TEE2", "TEE2_real", 1);
    init_symlink_info(&symlinks[7], "tee1", "tee1_real", 1);
    init_symlink_info(&symlinks[8], "tee2", "tee2_real", 1);
    init_symlink_info(&symlinks[9], "lk", "lk_real", 1);

    for (size_t i = 0; i < sizeof(symlinks) / sizeof(symlinks[0]); ++i) {
        const symlink_info_t *info = &symlinks[i];
        LOG(ERROR) << "Processing symlink: src=" << info->src_name << ", dst=" << info->dst_name << ", redirect=" << info->redirect_to_null;
        const char *resolved_symlink = resolve_symlink(info->src_name);
        if (resolved_symlink) {
            LOG(ERROR) << "Creating symlink from " << info->src_name << " to " << info->dst_name;
            REMOVE_SYMLINK(info->src_name);
            CREATE_SYMLINK(resolved_symlink, info->dst_name);

            if (info->redirect_to_null) {
                LOG(ERROR) << "Redirecting " << info->src_name << " to /dev/null";
                CREATE_SYMLINK("/dev/null", info->src_name);
            }
        } else {
            LOG(ERROR) << "Failed to resolve symlink for " << info->src_name;
        }
    }
}
