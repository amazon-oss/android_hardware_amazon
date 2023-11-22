#include <system/window.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <gui/SurfaceControl.h>


namespace android {

// ---------------------------------------------------------------------------

extern "C" {
void _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEjj(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags,
        SurfaceControl* parent,
        uint32_t windowType,
        uint32_t ownerUid);

void _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8Ejjij(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags)
{
return _ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEjj(name, w, h, format, flags, nullptr, 0, 0);
}

}

// ---------------------------------------------------------------------------

}; // namespace android
