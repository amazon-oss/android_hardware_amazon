#include <stdarg.h>
#include <string.h>

#define LOG_TAG "libamazonlog"
#define LOG_BUF_SIZE 1024
#define NEW_TAG_SIZE 128

#include <log/log.h>

int lab126_log_write(int bufID __unused, int prio __unused, const char *tag,
                     const char *fmt, ...) {
  char buf[LOG_BUF_SIZE];
  char new_tag[NEW_TAG_SIZE];

  if (!tag)
    tag = "";

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
  va_end(ap);

  snprintf(new_tag, sizeof(new_tag), "AMZ-%s", tag);
  return __android_log_buf_write(LOG_ID_MAIN, ANDROID_LOG_DEBUG, new_tag, buf);
}
