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

#include "init_charger.h"

char* read_cmdline(const char* key) {
    static char value[LINE_SIZE];
    FILE* cmdline = fopen(CMDLINE_PATH, "r");

    if (!cmdline) {
        perror("Failed to open " CMDLINE_PATH);
        return NULL;
    }

    char line[LINE_SIZE];
    while (fgets(line, sizeof(line), cmdline)) {
        char* token = strtok(line, " ");

        while (token != NULL) {
            if (strstr(token, key) == token && token[strlen(key)] == '=') {
                strncpy(value, token + strlen(key) + 1, LINE_SIZE - 1);
                value[LINE_SIZE - 1] = '\0';
                fclose(cmdline);
                return value;
            }
            token = strtok(NULL, " ");
        }
    }

    fclose(cmdline);
    return NULL;
}

void init_charger_properties() {
    const char* boot_reason = read_cmdline("androidboot.bootreason");

    if (boot_reason) {
        ERROR("Found androidboot.bootreason: %s\n", boot_reason);
        
        if (strcmp(boot_reason, "usb") == 0) {
            ERROR("boot_reason is 'usb', setting ro.bootmode to 'charger'\n");
            property_set("ro.bootmode", "charger");
        } else {
            ERROR("boot_reason is not 'usb', no action taken\n");
        }
    } else {
        ERROR("androidboot.bootreason not found, checking boot_reason\n");
        
        boot_reason = read_cmdline("boot_reason");
        
        if (boot_reason) {
            ERROR("Found boot_reason: %s\n", boot_reason);
            
            if (atoi(boot_reason) == 1) {
                ERROR("boot_reason is '1', setting ro.bootmode to 'charger'\n");
                property_set("ro.bootmode", "charger");
            } else {
                ERROR("boot_reason is not '1', no action taken\n");
            }
        } else {
            ERROR("boot_reason not found, no action taken\n");
        }
    }
}
