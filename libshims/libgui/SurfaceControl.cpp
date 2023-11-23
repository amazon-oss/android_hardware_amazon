#include <gui/SurfaceComposerClient.h>
#include <gui/SurfaceControl.h>

using android::status_t;

extern "C" status_t
_ZN7android14SurfaceControl4showEv(android::SurfaceControl *thisptr) {
  return (thisptr != nullptr) ? android::SurfaceComposerClient::Transaction()
                                    .show(thisptr)
                                    .apply()
                              : android::BAD_VALUE;
}

extern "C" status_t
_ZN7android14SurfaceControl4hideEv(android::SurfaceControl *thisptr) {
  return (thisptr != nullptr) ? android::SurfaceComposerClient::Transaction()
                                    .hide(thisptr)
                                    .apply()
                              : android::BAD_VALUE;
}

extern "C" status_t
_ZN7android14SurfaceControl8setLayerEi(android::SurfaceControl *thisptr,
                                       int32_t layer) {
  return (thisptr != nullptr) ? android::SurfaceComposerClient::Transaction()
                                    .setLayer(thisptr, layer)
                                    .apply()
                              : android::BAD_VALUE;
}
