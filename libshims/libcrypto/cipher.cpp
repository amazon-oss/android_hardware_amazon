#include <openssl/cipher.h>

extern "C" {
#include <openssl/rsa.h>
}

extern "C" int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                   int *out_len);

extern "C" int EVP_EncryptFinal(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                int *out_len) {
  int result = EVP_EncryptFinal_ex(ctx, out, out_len);
  EVP_CIPHER_CTX_cleanup(ctx);
  return result;
}
