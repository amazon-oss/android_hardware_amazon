#include <exception>
#include <new>
#include <string>

extern "C" {

const char *_ZNKSt17__Named_exception4whatEv(const std::exception *e) {
    return e->what();
}

const char *_ZNKSt9exception4whatEv(const std::exception *e) {
    return e->what();
}

void *_ZNSt17__Named_exceptionC2ERKS_(void *self, const void *other) {
    return new (self) std::exception(*reinterpret_cast<const std::exception *>(other));
}

void* _ZNSt17__Named_exceptionC2ERKSs(void* self, const void* /*str*/) {
    return new (self) std::exception();
}
}
