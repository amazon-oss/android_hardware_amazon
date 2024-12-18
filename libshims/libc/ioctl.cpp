#include <errno.h>
#include <log/log.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <unistd.h>

extern "C" int iactl(int fd, unsigned long request, ...) {
    va_list args;
    void *arg;

    va_start(args, request);
    arg = va_arg(args, void *);
    va_end(args);

    return syscall(SYS_ioctl, fd, request, arg);
}
