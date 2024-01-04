#include <utils/Thread.h>

using android::status_t;

#if defined(__LP64__)
extern "C" status_t _ZN7android6Thread3runEPKcim(void *thisptr,
                                                 const char *name,
                                                 int32_t priority,
                                                 size_t stack);

extern "C" status_t _ZN7android6Custom3runEPKcim(void *thisptr,
                                                 const char *name,
                                                 int32_t priority,
                                                 size_t stack) {
  return _ZN7android6Thread3runEPKcim(thisptr, name ? name : "",
                                      priority, stack);
}

#else
extern "C" status_t _ZN7android6Thread3runEPKcij(void *thisptr,
                                                 const char *name,
                                                 int32_t priority,
                                                 size_t stack);

extern "C" status_t _ZN7android6Custom3runEPKcij(void *thisptr,
                                                 const char *name,
                                                 int32_t priority,
                                                 size_t stack) {
  return _ZN7android6Thread3runEPKcij(thisptr, name ? name : "",
                                      priority, stack);
}
#endif
