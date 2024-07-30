#include <media/IMediaSource.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaSource.h>

extern "C" android::MediaBufferGroup *_ZN7android16MediaBufferGroupC1Ev() {
    return new android::MediaBufferGroup(0);
}

extern "C" int
_ZN7android16MediaBufferGroup14acquire_bufferEPPNS_11MediaBufferEb(
    void *obj, android::MediaBuffer **out, bool nonBlocking,
    size_t requestedSize) {
    android::MediaBufferGroup *mbg =
        static_cast<android::MediaBufferGroup *>(obj);
    return mbg->acquire_buffer(out, nonBlocking, requestedSize);
}

extern "C" void
_ZNK7android11MediaSource11ReadOptions9getSeekToEPxPNS1_8SeekModeE(
    void *obj, int64_t time_us,
    android::MediaSource::ReadOptions::SeekMode mode) {
    android::IMediaSource::ReadOptions *rop =
        static_cast<android::IMediaSource::ReadOptions *>(obj);
    rop->setSeekTo(time_us, mode);
}

extern "C" bool
_ZNK7android11MediaSource11ReadOptions14getNonBlockingEv(void *obj) {
    android::IMediaSource::ReadOptions *rop =
        static_cast<android::IMediaSource::ReadOptions *>(obj);
    return rop->getNonBlocking();
}
