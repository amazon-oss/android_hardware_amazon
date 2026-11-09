#include <stdlib.h>

void CONF_modules_unload(int all __attribute__((unused))) {
}

void OBJ_cleanup(void) {
}

void CRYPTO_free(void *ptr) {
    free(ptr);
}
