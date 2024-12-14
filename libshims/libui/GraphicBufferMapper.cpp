#include <gui/SurfaceControl.h>
#include <system/window.h>
#include <ui/Rect.h>

namespace android {

extern "C" {

status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv(
    void *thisptr, buffer_handle_t handle, uint32_t usage, const Rect &bounds,
    void **vaddr);

status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handleiRKNS_4RectEPPv(
    void *thisptr, buffer_handle_t handle, int usage,
    const android::Rect &bounds, void **vaddr) {
    return _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv(
        thisptr, handle, (uint32_t)usage, bounds, vaddr);
}
}

extern "C" {
status_t
_ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcr(
    void *thisptr, buffer_handle_t handle, uint32_t usage, const Rect &bounds,
    android_ycbcr *ycbcr);
status_t _ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handleiRKNS_4RectEP13android_ycbcr(
    void *thisptr, buffer_handle_t handle, int usage, const Rect &bounds,
    android_ycbcr *ycbcr) {
  return _ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcr(
      thisptr, handle, static_cast<uint32_t>(usage), bounds, ycbcr);
}
}

}; // namespace android
