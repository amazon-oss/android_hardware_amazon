#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include <ui/GraphicBuffer.h>
#include <utils/Errors.h>

using android::PixelFormat;
using android::status_t;

extern "C" void *
_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
    void *thisptr, uint32_t inWidth, uint32_t inHeight, PixelFormat inFormat,
    uint32_t inUsage, std::string requestorName = "<Unknown>");

extern "C" void *_ZN7android13GraphicBufferC1Ejjij(void *thisptr,
                                                   uint32_t inWidth,
                                                   uint32_t inHeight,
                                                   PixelFormat inFormat,
                                                   uint32_t inUsage) {
  return _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
      thisptr, inWidth, inHeight, inFormat, inUsage);
}

extern "C" void *_ZN7android13GraphicBufferC1EjjijjjP13native_handleb(
    void *thisptr, const native_handle_t *handle,
    android::GraphicBuffer::HandleWrapMethod method, uint32_t width,
    uint32_t height, int format, uint32_t layerCount, uint64_t usage,
    uint32_t stride);

extern "C" void *_ZN7android13GraphicBufferC1EjjijjP13native_handleb(
    void *thisptr, uint32_t inWidth, uint32_t inHeight, int inFormat,
    uint32_t inUsage, uint32_t inStride, native_handle_t *inHandle,
    bool keepOwnership) {
  android::GraphicBuffer::HandleWrapMethod inMethod =
      (keepOwnership ? android::GraphicBuffer::TAKE_HANDLE
                     : android::GraphicBuffer::WRAP_HANDLE);
  return _ZN7android13GraphicBufferC1EjjijjjP13native_handleb(
      thisptr, inHandle, inMethod, inWidth, inHeight, inFormat,
      static_cast<uint32_t>(1), static_cast<uint64_t>(inUsage), inStride);
}

extern "C" status_t _ZN7android13GraphicBuffer4lockEjPPvPiS3_(
    void *thisptr, uint32_t inUsage, void **vaddr, int32_t *outBytesPerPixel,
    int32_t *outBytesPerStride);

extern "C" status_t _ZN7android13GraphicBuffer4lockEjPPv(void *thisptr,
                                                         uint32_t inUsage,
                                                         void **vaddr) {
  return _ZN7android13GraphicBuffer4lockEjPPvPiS3_(thisptr, inUsage, vaddr,
                                                   nullptr, nullptr);
}
