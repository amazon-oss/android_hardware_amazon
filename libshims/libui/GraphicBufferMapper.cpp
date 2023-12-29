#include <stdint.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>

using android::Rect;
using android::status_t;

extern "C" {
status_t
_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPvPiS9_(
    void *thisptr, buffer_handle_t handle, uint32_t usage, const Rect &bounds,
    void **vaddr, int32_t *outBytesPerPixel, int32_t *outBytesPerStride);

status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv(
    void *thisptr, buffer_handle_t handle, uint32_t usage, const Rect &bounds,
    void **vaddr) {
  return _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPvPiS9_(
      thisptr, handle, usage, bounds, vaddr, nullptr, nullptr);
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
