#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <exception>
#include <new>

struct NamedException {
    void *vptr;
    char static_name[256];
    char *name;
};

struct StlportString {
    char static_buf[16];
    char *finish;
    char *start;
};

static_assert(offsetof(NamedException, name) == 260, "__Named_exception layout");
static_assert(offsetof(StlportString, start) == 20, "stlport string layout");

static void init_name(NamedException *e, const char *str) {
    size_t size = strlen(str) + 1;

    if (size > sizeof(e->static_name)) {
        e->name = static_cast<char *>(malloc(size));
        if (e->name) {
            *reinterpret_cast<size_t *>(e->static_name) = size;
        } else {
            size = sizeof(e->static_name);
            e->name = e->static_name;
        }
    } else {
        e->name = e->static_name;
    }

    strncpy(e->name, str, size - 1);
    e->name[size - 1] = '\0';
}

extern "C" {

const char *_ZNKSt17__Named_exception4whatEv(const NamedException *e) {
    return e->name;
}

void *_ZNSt17__Named_exceptionC2ERKS_(NamedException *self, const NamedException *other) {
    new (self) std::exception();
    init_name(self, other->name);
    return self;
}

void *_ZNSt17__Named_exceptionC2ERKSs(NamedException *self, const StlportString *str) {
    new (self) std::exception();
    init_name(self, str->start);
    return self;
}
}
