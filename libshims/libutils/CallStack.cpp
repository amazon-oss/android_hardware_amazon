#include <stdint.h>
#include <sys/types.h>

extern "C" void* _ZN7android9CallStackC1EPKci(void* thisptr, const char* logtag,
                                              int32_t ignoreDepth);

extern "C" void _ZN7android9CallStack6updateEii(void* thisptr, int32_t ignoreDepth,
                                                int32_t maxDepth);

extern "C" void _ZN7android9CallStack6updateEiii(void* thisptr, int32_t ignoreDepth,
                                                 int32_t maxDepth, pid_t tid __unused) {
    _ZN7android9CallStack6updateEii(thisptr, ignoreDepth, maxDepth);
}

extern "C" void* _ZN7android9CallStackC1EPKcii(void* thisptr, const char* logtag,
                                               int32_t ignoreDepth, int32_t maxDepth __unused) {
    return _ZN7android9CallStackC1EPKci(thisptr, logtag, ignoreDepth);
}
