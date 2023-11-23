#include <SkBitmap.h>

namespace android {

extern "C" void
_ZN8SkBitmap11allocPixelsEPNS_9AllocatorE(SkBitmap *thisptr,
                                          SkBitmap::Allocator *allocator);

extern "C" void _ZN8SkBitmap11allocPixelsEPNS_9AllocatorEP12SkColorTable(
    SkBitmap *thisptr, SkBitmap::Allocator *allocator, void *ctable __unused) {
  _ZN8SkBitmap11allocPixelsEPNS_9AllocatorE(thisptr, allocator);
}

}
