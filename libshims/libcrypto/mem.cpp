#include <openssl/mem.h>

extern "C" {

#undef CRYPTO_malloc
#undef CRYPTO_realloc
#undef CRYPTO_free

void *CRYPTO_malloc(size_t size) { return OPENSSL_malloc(size); }

void *CRYPTO_realloc(void *ptr, size_t new_size) {
  return OPENSSL_realloc(ptr, new_size);
}

void CRYPTO_free(void *ptr) { return OPENSSL_free(ptr); }
}
