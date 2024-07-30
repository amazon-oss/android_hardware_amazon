#include <new>

#include <media/IMediaSource.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaSource.h>

extern "C" void *_ZN7android16MediaBufferGroupC1Ev(void *obj) {
    return new (obj) android::MediaBufferGroup(0);
}

extern "C" int
_ZN7android16MediaBufferGroup14acquire_bufferEPPNS_11MediaBufferEb(
    void *obj, android::MediaBuffer **out, bool nonBlocking) {
    android::MediaBufferGroup *mbg =
        static_cast<android::MediaBufferGroup *>(obj);
    return mbg->acquire_buffer(out, nonBlocking, 0);
}

extern "C" bool
_ZNK7android11MediaSource11ReadOptions9getSeekToEPxPNS1_8SeekModeE(
    const void *obj, int64_t *time_us,
    android::IMediaSource::ReadOptions::SeekMode *mode) {
    const android::IMediaSource::ReadOptions *rop =
        static_cast<const android::IMediaSource::ReadOptions *>(obj);
    return rop->getSeekTo(time_us, mode);
}

extern "C" bool
_ZNK7android11MediaSource11ReadOptions14getNonBlockingEv(const void *obj) {
    const android::IMediaSource::ReadOptions *rop =
        static_cast<const android::IMediaSource::ReadOptions *>(obj);
    return rop->getNonBlocking();
}
