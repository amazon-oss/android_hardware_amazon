#include <binder/IInterface.h>

extern "C" void _ZN7android10IInterface8asBinderEv(void *retval, void* self) {
    android::sp<android::IBinder> *ret = new(retval) android::sp<android::IBinder>;
    *ret = android::IInterface::asBinder((android::IInterface*)self);
}

extern "C" void _ZNK7android10IInterface8asBinderEv(void *retval, void *self) {
    android::sp<android::IBinder> *ret = new(retval) android::sp<android::IBinder>;
    *ret = android::IInterface::asBinder((android::IInterface*)self);
}

#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

extern "C" android::sp<android::IBinder> _ZN7lineage10IInterface8asBinderEv(android::IInterface* self) {
    return android::IInterface::asBinder(self);
}
