#include <stdint.h>
#include <sys/types.h>

extern "C" int _ZN7android15requestPriorityEiiibb(pid_t pid, pid_t tid, int32_t prio, bool isForApp, bool asynchronous);

extern "C" int _ZN7android15requestPriorityEiiib(pid_t pid, pid_t tid, int32_t prio, bool asynchronous) {
    return _ZN7android15requestPriorityEiiibb(pid, tid, prio, false, asynchronous);
}
