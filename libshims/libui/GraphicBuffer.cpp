#include <string.h>
#include <sys/types.h>

#include <ui/GraphicBuffer.h>

using android::PixelFormat;

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
