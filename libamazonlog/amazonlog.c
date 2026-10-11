/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/cdefs.h>

#include <log/log.h>

typedef enum {
    METRICS_PRIORITY_HIGH,
    METRICS_PRIORITY_NORMAL,
    METRICS_PRIORITY_COUNTER,
    METRICS_PRIORITY_TIMER,
    METRICS_PRIORITY_DISCRETE,
    METRICS_PRIORITY_NONE,
} metrics_prio_t;

typedef enum {
    VITALS_EXT_DIMENSION,
    VITALS_EXT_META,
    VITALS_EXT_MAX,
} vitals_ext_data_type;

typedef enum {
    VITALS_TYPE_1,
    VITALS_TYPE_2,
} vitals_type;

#define VITALS_EXT_STR_LEN 101
#define VITALS_EXT_MAX_PAIRS 12

typedef struct {
    char key[VITALS_EXT_STR_LEN];
    char val[VITALS_EXT_STR_LEN];
} vitals_ext_pair;

typedef struct {
    size_t count[VITALS_EXT_MAX];
    vitals_ext_pair data[VITALS_EXT_MAX][VITALS_EXT_MAX_PAIRS];
} vitals_extended_data;

_Static_assert(sizeof(vitals_ext_pair) == 202, "vitals_ext_pair layout");
_Static_assert(offsetof(vitals_extended_data, data) == 2 * sizeof(size_t), "count layout");
_Static_assert(sizeof(((vitals_extended_data*)0)->data[0]) == 2424, "data layout");

static const char* const prio_names[] = {"HI", "NR", "CT", "TI", "DV", "NA"};
static const size_t ext_max[VITALS_EXT_MAX] = {5, 12};

const char* metrics_log_prio_to_name(int prio) {
    if ((unsigned int)prio > METRICS_PRIORITY_NONE) prio = METRICS_PRIORITY_NORMAL;
    return prio_names[prio];
}

int metrics_prio_name_to_metrics_prio(const char* name) {
    const char* base;
    int i;

    if (!name) return -1;

    base = strrchr(name, '/');
    if (base) name = base + 1;

    for (i = METRICS_PRIORITY_HIGH; i <= METRICS_PRIORITY_NONE; i++)
        if (!strcmp(name, prio_names[i])) return i;

    return -1;
}

void __vitals_log_ext_set_data(vitals_extended_data* ext, const char* key, const char* val,
                               vitals_ext_data_type type) {
    vitals_ext_pair* pair;

    if (!ext || (unsigned int)type >= VITALS_EXT_MAX || !key || !val) return;
    if (ext->count[type] >= ext_max[type]) return;

    pair = &ext->data[type][ext->count[type]];
    strncpy(pair->key, key, VITALS_EXT_STR_LEN - 1);
    pair->key[VITALS_EXT_STR_LEN - 1] = '\0';
    strncpy(pair->val, val, VITALS_EXT_STR_LEN - 1);
    pair->val[VITALS_EXT_STR_LEN - 1] = '\0';
    ext->count[type]++;
}

int __metrics_log_print(int prio __unused, const char* tag __unused, int mprio __unused,
                        const char* domain __unused, const char* source __unused,
                        const char* fmt __unused, ...) {
    return 0;
}

int __metrics_log_print_v2(int buf_id __unused, const char* tag __unused,
                           const char* program __unused, const char* source __unused,
                           int prio __unused, const char* fmt __unused, ...) {
    return 0;
}

int __engagement_metrics_log_print(int buf_id __unused, const char* tag __unused,
                                   int prio __unused, const char* group_id __unused,
                                   const char* schema_id __unused, const char* fmt __unused, ...) {
    return 0;
}

int __vitals_log_print(const char* tag __unused, const char* program __unused,
                       const char* source __unused, const char* key __unused, double cv __unused,
                       const char* unit __unused, const char* metadata __unused,
                       vitals_type type __unused, int is_counter __unused) {
    return 0;
}

int __vitals_log_ext_print(const char* tag __unused, const char* program __unused,
                           const char* source __unused, const char* key __unused,
                           double cv __unused, const char* unit __unused,
                           const char* metadata __unused,
                           const vitals_extended_data* ext __unused, vitals_type type __unused,
                           int is_counter __unused) {
    return 0;
}

int __vitals_log_ext_print_v2(const char* tag __unused, const char* group_id __unused,
                              const char* schema_id __unused, const char* program __unused,
                              const char* source __unused, const char* key __unused,
                              double cv __unused, const char* unit __unused,
                              const char* metadata __unused,
                              const vitals_extended_data* ext __unused,
                              vitals_type type __unused, int is_counter __unused) {
    return 0;
}

static int lab126_vlog(int prio, const char* tag, const char* fmt, va_list ap) {
    char msg[1024];

    if (prio < ANDROID_LOG_VERBOSE || prio > ANDROID_LOG_FATAL) prio = ANDROID_LOG_DEBUG;

    vsnprintf(msg, sizeof(msg), fmt, ap);
    return __android_log_buf_write(LOG_ID_MAIN, prio, tag ? tag : "", msg);
}

int lab126_log_write(int buf_id __unused, int prio __unused, const char* tag, const char* fmt,
                     ...) {
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = lab126_vlog(ANDROID_LOG_DEBUG, tag, fmt, ap);
    va_end(ap);
    return ret;
}

int lab126_log_write2(int prio, const void* ctx __unused, int type __unused, const char* tag,
                      const char* fmt, ...) {
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = lab126_vlog(prio, tag, fmt, ap);
    va_end(ap);
    return ret;
}
