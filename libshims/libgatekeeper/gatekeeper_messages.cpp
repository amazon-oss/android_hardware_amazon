#include <gatekeeper/gatekeeper_messages.h>
#include <stdint.h>

extern "C" void *_ZN10gatekeeper13EnrollRequestC1EjPNS_11SizedBufferES2_S2_(
    void *thisptr, uint32_t user_id, gatekeeper::SizedBuffer *password_handle,
    gatekeeper::SizedBuffer *provided_password,
    gatekeeper::SizedBuffer *enrolled_password) {
  new (thisptr) gatekeeper::EnrollRequest(user_id, std::move(*password_handle),
                                          std::move(*provided_password),
                                          std::move(*enrolled_password));
  return thisptr;
}
