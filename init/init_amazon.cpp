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

#include <fcntl.h>
#include <private/android_filesystem_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "init_amazon.h"
#include "init_amonet.h"

#include <sys/resource.h>

#define CPUINFO_PATH "/proc/cpuinfo"
#define BOOTMODE_PATH "/sys/devices/virtual/BOOT/BOOT/boot/boot_mode"

#define BUF_SIZE 64
#define STRCONV_(x) #x
#define STRCONV(x) "%" STRCONV_(x) "s"

__attribute__((weak)) void init_amazon_properties(int rev) { UNUSED(rev); }

extern void init_amonet_symlinks();

static int read_bootmode() {
    int fd = -1;
    char buffer[16];

    fd = open(BOOTMODE_PATH, O_RDONLY);
    if (fd < 0) {
        ERROR("Failed to open boot mode file");
        exit(EXIT_FAILURE);
    }

    int bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes < 0) {
        ERROR("Failed to read from boot mode file");
        return 0; // Assume normal boot mode
    }

    buffer[bytes] = '\0';
    return atoi(buffer);
}

static int read_revision() {
    FILE *cpuinfo = fopen(CPUINFO_PATH, "r");
    if (!cpuinfo) {
        ERROR("Unable to open %s!\n", CPUINFO_PATH);
        return -1;
    }

    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), cpuinfo)) {
        if (strncmp(line, "Revision", 8) == 0) {
            fclose(cpuinfo);
            char *rev_str = strchr(line, ':');
            if (rev_str) {
                rev_str++;
                while (*rev_str == ' ' || *rev_str == '\t')
                    rev_str++;
                return (int)strtol(rev_str, NULL, 16);
            }
            break;
        }
    }
    fclose(cpuinfo);
    return -1;
}

void vendor_load_properties() {
    int rev = read_revision();
    int bmd = read_bootmode();
    ERROR("Detected Amazon device REV=%d\n", rev);
    if (bmd != 2) {
        init_amazon_properties(rev);
        // TWRP itself takes care of symlinks
        init_amonet_symlinks();
    }
}
