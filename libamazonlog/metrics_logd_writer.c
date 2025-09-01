// vendor/amazon/libamazonlog/libamazonlog.c
// Purpose: Vendor-forwarding shim for Amazon metrics/vitals symbols.
// Behavior: Export required symbols and forward to liblog with stable formatting.
// Build: shared vendor lib; see Android.bp next to this file.

#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define LOG_TAG "libamazonlog"
#include <log/log.h>  // __android_log_print, __android_log_vprint

// Map vendor priorities to Android log priorities.
static inline int to_android_prio(int p) {
  switch (p) {
    case 0: return ANDROID_LOG_ERROR;   // METRICS_PRIORITY_HIGH
    case 1: return ANDROID_LOG_INFO;    // METRICS_PRIORITY_NORMAL
    case 2: return ANDROID_LOG_INFO;    // COUNTER
    case 3: return ANDROID_LOG_DEBUG;   // TIMER
    case 4: return ANDROID_LOG_DEBUG;   // DISCRETE
    default: return ANDROID_LOG_INFO;
  }
}

__attribute__((visibility("default")))
int __metrics_log_print_v2(int prio, const char* tag, int mpriority,
                           const char* domain, const char* source,
                           const char* fmt, ...) {
  // Forward vendor metrics log to liblog with a compact prefix.
  const int ap = to_android_prio(mpriority);
  va_list args;
  va_start(args, fmt);

  // Compose a domain/source prefix without copying the full message.
  // Example prefix: [metrics d=audio s=asp] real message...
  int rc = __android_log_print(ap,
                               tag && tag[0] ? tag : LOG_TAG,
                               "[metrics d=%s s=%s] ",
                               domain ? domain : "-",
                               source ? source : "-");
  // Print the original message body.
  rc = __android_log_vprint(ap,
                            tag && tag[0] ? tag : LOG_TAG,
                            fmt ? fmt : "",
                            args);
  va_end(args);
  return rc;
}

__attribute__((visibility("default")))
int __metrics_log_print(int prio, const char* tag, int mpriority,
                        const char* domain, const char* source,
                        const char* fmt, ...) {
  const int ap = to_android_prio(mpriority);
  va_list args;
  va_start(args, fmt);
  int rc = __android_log_print(ap,
                               tag && tag[0] ? tag : LOG_TAG,
                               "[metrics d=%s s=%s] ",
                               domain ? domain : "-",
                               source ? source : "-");
  rc = __android_log_vprint(ap,
                            tag && tag[0] ? tag : LOG_TAG,
                            fmt ? fmt : "",
                            args);
  va_end(args);
  return rc;
}

// --- Vitals shim: emit structured JSON to logcat --------------------------------

typedef enum {
  VITALS_EXT_DIMENSION,
  VITALS_EXT_META,
  VITALS_EXT_MAX
} vitals_ext_data_type;

typedef struct {
  char key[64];
  char val[64];
} vitals_key_value_pair;

typedef struct {
  vitals_ext_data_type ext_type[VITALS_EXT_MAX];
  size_t ext_size[VITALS_EXT_MAX];
} vitals_extended_data;

typedef enum { VITALS_TYPE_1, VITALS_TYPE_2 } vitals_type;

__attribute__((visibility("default")))
const char* metrics_log_prio_to_name(int /*mprio*/) { return "NORMAL"; }

__attribute__((visibility("default")))
int metrics_prio_name_to_metrics_prio(const char* /*name*/) { return 1; }

static inline int log_json(const char* tag, const char* lvl, const char* key,
                           double cv, const char* unit, const char* metadata,
                           const char* extra) {
  return __android_log_print(ANDROID_LOG_INFO,
                             tag && tag[0] ? tag : LOG_TAG,
                             "{\"type\":\"vitals\",\"lvl\":\"%s\",\"key\":\"%s\","
                             "\"cv\":%.6f,\"unit\":\"%s\",\"meta\":\"%s\"%s}",
                             lvl ? lvl : "N",
                             key ? key : "-",
                             cv,
                             unit ? unit : "-",
                             metadata ? metadata : "-",
                             extra ? extra : "");
}

__attribute__((visibility("default")))
int __vitals_log_print(const char* tag, const char* /*program*/,
                       const char* /*source*/, const char* key, double cv,
                       const char* unit, const char* metadata, vitals_type type,
                       int is_counter) {
  char extra[64];
  snprintf(extra, sizeof(extra), ",\"t\":%d,\"ctr\":%d", (int)type, is_counter);
  return log_json(tag, "N", key, cv, unit, metadata, extra);
}

__attribute__((visibility("default")))
int __vitals_log_ext_print(const char* tag, const char* /*program*/,
                           const char* /*source*/, const char* key, double cv,
                           const char* unit, const char* metadata,
                           const vitals_extended_data* ext_data,
                           vitals_type type, int is_counter) {
  char extra[80];
  size_t ext0 = ext_data ? ext_data->ext_size[0] : 0;
  size_t ext1 = ext_data ? ext_data->ext_size[1] : 0;
  snprintf(extra, sizeof(extra),
           ",\"t\":%d,\"ctr\":%d,\"ext0\":%zu,\"ext1\":%zu",
           (int)type, is_counter, ext0, ext1);
  return log_json(tag, "E", key, cv, unit, metadata, extra);
}

__attribute__((visibility("default")))
void __vitals_log_ext_print_v2(uint32_t log_level, const char* tag,
                               const char* /*program*/, const char* /*source*/,
                               const char* key, double cv, const char* unit,
                               const char* metadata, uint64_t event_timestamp,
                               uint32_t process_id, const unsigned char* log_flag,
                               const vitals_extended_data* ext_data,
                               vitals_type type, int is_counter,
                               uint32_t thread_id, uint32_t user_id) {
  size_t ext0 = ext_data ? ext_data->ext_size[0] : 0;
  size_t ext1 = ext_data ? ext_data->ext_size[1] : 0;
  __android_log_print(ANDROID_LOG_INFO,
                      tag && tag[0] ? tag : LOG_TAG,
                      "{\"type\":\"vitals2\",\"lvl\":%u,\"key\":\"%s\",\"cv\":%.6f,"
                      "\"unit\":\"%s\",\"meta\":\"%s\",\"ts\":%llu,\"pid\":%u,"
                      "\"tid\":%u,\"uid\":%u,\"flag\":%u,\"t\":%d,\"ctr\":%d,"
                      "\"ext0\":%zu,\"ext1\":%zu}",
                      (unsigned)log_level,
                      key ? key : "-",
                      cv,
                      unit ? unit : "-",
                      metadata ? metadata : "-",
                      (unsigned long long)event_timestamp,
                      (unsigned)process_id,
                      (unsigned)thread_id,
                      (unsigned)user_id,
                      (unsigned)(log_flag ? *log_flag : 0),
                      (int)type,
                      is_counter,
                      ext0, ext1);
}

__attribute__((visibility("default")))
void __vitals_log_ext_set_data(vitals_extended_data* /*ext_data*/,
                               const char* /*key*/, const char* /*val*/,
                               vitals_ext_data_type /*type*/) {
  // No backing store here; this shim only forwards logs.
}
