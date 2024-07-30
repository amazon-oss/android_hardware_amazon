#include <stdarg.h>
#include <string.h>

#define LOG_TAG "libamazonlog"
#define LOG_BUF_SIZE 1024
#define NEW_TAG_SIZE 128

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

const char *flooding_tags[] = {"wmt_loader"};

#include <log/log.h>

int lab126_log_write(int bufID __unused, int prio __unused, const char *tag,
                     const char *fmt, ...) {
    char buf[LOG_BUF_SIZE];
    char new_tag[NEW_TAG_SIZE];

    if (!tag)
        tag = "";

    for (size_t i = 0; i < ARRAY_SIZE(flooding_tags); ++i) {
        if (strcmp(tag, flooding_tags[i]) == 0) {
            return 0;
        }
    }

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    snprintf(new_tag, sizeof(new_tag), "AMZ-%s", tag);
    return __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_DEBUG, new_tag,
                                   buf);
}
