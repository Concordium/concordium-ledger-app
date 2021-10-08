#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t

#include "sha256.h"

// This is a mock to replace the sha256, which the app uses, during testing.
void *
__wrap_cx_hash_sha256 (uint8_t* buffer, size_t length, uint8_t* hash, size_t *hashLength)
{
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, buffer, length);
    sha256_final(&ctx, hash);
    return 0;
}


