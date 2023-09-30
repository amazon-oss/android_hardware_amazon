#include <stdarg.h>
#include <string.h>

#define LOG_TAG "libamazonlog"
#define LOG_BUF_SIZE 1024

#include <log/log.h>

typedef enum {
    METRICS_PRIORITY_HIGH,
    METRICS_PRIORITY_NORMAL,
    METRICS_PRIORITY_COUNTER,
    METRICS_PRIORITY_TIMER,
    METRICS_PRIORITY_DISCRETE,
    METRICS_PRIORITY_MAX
} metrics_prio_t;

typedef enum { VITALS_EXT_DIMENSION, VITALS_EXT_META, VITALS_EXT_MAX } vitals_ext_data_type;

typedef struct {
    char key[64];
    char val[64];
} vitals_key_value_pair;

typedef struct {
    vitals_ext_data_type ext_type[VITALS_EXT_MAX];
    size_t ext_size[VITALS_EXT_MAX];
} vitals_extended_data;

typedef enum { VITALS_TYPE_1, VITALS_TYPE_2 } vitals_type;

const char* metrics_log_prio_to_name(metrics_prio_t mprio) {
    ALOGV("[%s]: mprio=%d", __func__, mprio);
    return "DUMMY";
}

metrics_prio_t metrics_prio_name_to_metrics_prio(const char* mprioName) {
    ALOGV("[%s]: mprioName=%s", __func__, mprioName);
    return METRICS_PRIORITY_NORMAL;
}

int __metrics_log_print(int prio, const char* tag, int mpriority, const char* domain,
                        const char* source, const char* fmt, ...) {
    ALOGV("[%s]: prio=%d, tag=%s, mpriority=%d, domain=%s, source=%s, fmt=%s", __func__, prio, tag,
          mpriority, domain, source, fmt);
    return 0;
}

int __vitals_log_print(const char* tag, const char* program, const char* source, const char* key,
                       double cv, const char* unit, const char* metadata, vitals_type type,
                       int is_counter) {
    ALOGV("[%s]: tag=%s, program=%s, source=%s, key=%s, cv=%f, unit=%s, "
          "metadata=%s, type=%d, is_counter=%d",
          __func__, tag, program, source, key, cv, unit, metadata, type, is_counter);
    return 0;
}

int __vitals_log_ext_print(const char* tag, const char* program, const char* source,
                           const char* key, double cv, const char* unit, const char* metadata,
                           const vitals_extended_data* ext_data, vitals_type type, int is_counter) {
    ALOGV("[%s]: tag=%s, program=%s, source=%s, key=%s, cv=%f, unit=%s, "
          "metadata=%s, ext_data=%p, type=%d, is_counter=%d",
          __func__, tag, program, source, key, cv, unit, metadata, ext_data, type, is_counter);
    return 0;
}

void __vitals_log_ext_print_v2(uint32_t log_level, const char* tag, const char* program,
                               const char* source, const char* key, double cv, const char* unit,
                               const char* metadata, uint64_t event_timestamp, uint32_t process_id,
                               const unsigned char* log_flag, const vitals_extended_data* ext_data,
                               vitals_type type, int is_counter, uint32_t thread_id,
                               uint32_t user_id) {
    ALOGV("[%s]: log_level=%u, tag=%s, program=%s, source=%s, key=%s, cv=%f, "
          "unit=%s, metadata=%s, event_timestamp=%llu, process_id=%u, "
          "log_flag=%u, ext_data_count=%zu, type=%d, is_counter=%d, "
          "thread_id=%u, user_id=%u",
          __func__, log_level, tag ? tag : "N/A", program ? program : "N/A",
          source ? source : "N/A", key ? key : "N/A", cv, unit ? unit : "N/A",
          metadata ? metadata : "N/A", event_timestamp, process_id, log_flag ? *log_flag : 0,
          ext_data ? ext_data->ext_size[0] : 0, type, is_counter, thread_id, user_id);
}

void __vitals_log_ext_set_data(vitals_extended_data* ext_data, const char* key, const char* val,
                               vitals_ext_data_type type) {
    ALOGV("[%s]: ext_data=%p, key=%s, val=%s, type=%d", __func__, ext_data, key, val, type);
}
