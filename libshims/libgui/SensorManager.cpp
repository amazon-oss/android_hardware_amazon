#include <stdlib.h>
#include <stdint.h>
#include <new>

#include <gui/SensorManager.h>
#include <gui/SensorEventQueue.h>

namespace android {

extern "C" SensorManager* _ZN7android9SingletonINS_13SensorManagerEE9sInstanceE = nullptr;
extern "C" Mutex _ZN7android9SingletonINS_13SensorManagerEE5sLockE(Mutex::PRIVATE);

extern "C" void _ZN7android13SensorManagerC1ERKNS_8String16E(void* obj, const String16& opPackageName);

extern "C" void _ZN7android13SensorManagerC1Ev(void* obj) {
    _ZN7android13SensorManagerC1ERKNS_8String16E(obj, String16());
}

extern "C" void _ZN7android13SensorManager16createEventQueueEv(void* ret, SensorManager* obj) {
    new (ret) sp<SensorEventQueue>(obj->createEventQueue());
}

extern "C" ssize_t _ZN7android13SensorManager13getSensorListEPPKPKNS_6SensorE(void* obj, const Sensor* const** list);

extern "C" ssize_t _ZNK7android13SensorManager13getSensorListEPPKPKNS_6SensorE(void* obj, const Sensor* const** list) {
    return _ZN7android13SensorManager13getSensorListEPPKPKNS_6SensorE(obj, list);
}

}
