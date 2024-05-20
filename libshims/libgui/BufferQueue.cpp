#include <gui/BufferQueue.h>

extern "C" void
_ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(
        void* thisptr, android::sp<android::IGraphicBufferProducer>* outProducer,
        android::sp<android::IGraphicBufferConsumer>* outConsumer, const android::sp<android::IGraphicBufferAlloc>& allocator);

extern "C" void _ZN7android11BufferQueueC1ERKNS_2spINS_19IGraphicBufferAllocEEE(
        void* thisptr, const android::sp<android::IGraphicBufferAlloc>& allocator) {
    android::sp<android::IGraphicBufferProducer> producer;
    android::sp<android::IGraphicBufferConsumer> consumer;
    _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(thisptr,
            &producer, &consumer, allocator);
}
