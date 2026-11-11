/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>
#include <string.h>

extern "C" int property_get(const char* key, char* value, const char* default_value) {
    static auto property_get_orig =
            reinterpret_cast<typeof(property_get)*>(dlsym(RTLD_NEXT, "property_get"));

    if (strcmp(key, "ro.product.manufacturer") == 0)
        return strlen(strcpy(value, "Amazon"));

    return property_get_orig(key, value, default_value);
}
