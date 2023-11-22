#include <system/window.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <gui/SurfaceControl.h>
#include <gui/SurfaceComposerClient.h>

android::SurfaceComposerClient::Transaction *t;

namespace android {

// ---------------------------------------------------------------------------

extern "C" {

void _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataEPj(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags,
        SurfaceControl* parent,
        LayerMetadata metadata,
        uint32_t* outTransformHint);

void _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEjj(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags,
        SurfaceControl* parent,
        uint32_t windowType,
        uint32_t ownerUid)
{
LayerMetadata metadata;
metadata.setInt32(METADATA_WINDOW_TYPE, windowType);
metadata.setInt32(METADATA_OWNER_UID, ownerUid);
return _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataEPj(name, w, h, format, flags, parent, metadata, nullptr);
}
void _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8Ejjij(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags)
{
return _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEjj(name, w, h, format, flags, nullptr, 0, 0);
}

void _ZN7android21SurfaceComposerClient21openGlobalTransactionEv() {
    t = new(android::SurfaceComposerClient::Transaction);
}

void _ZN7android21SurfaceComposerClient22closeGlobalTransactionEb(bool synchronous __unused)  {
    t->apply();
    delete t;
    t = nullptr;
}

}

// ---------------------------------------------------------------------------

}; // namespace android
