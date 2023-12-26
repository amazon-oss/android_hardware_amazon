#include <gui/BufferQueue.h>

using android::IBinder;
using android::IGraphicBufferConsumer;
using android::IGraphicBufferProducer;
using android::sp;

extern "C" void
_ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(
    sp<IGraphicBufferProducer> *outProducer,
    sp<IGraphicBufferConsumer> *outConsumer, void *) {
  // createBufferQueue is a static method, call it directly
  android::BufferQueue::createBufferQueue(outProducer, outConsumer, false);
}
