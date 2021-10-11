#ifndef _CONCORDIUM_TEST_CX_H_
#define _CONCORDIUM_TEST_CX_H_

#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t

void * cx_hash_sha256 (uint8_t* buffer, size_t length, uint8_t* hash, size_t hashLength);

typedef struct cx_sha256_s {
    size_t dummy;
} cx_sha256_t, cx_hash_t;

void cx_hash(cx_hash_t* ctx, uint8_t mode, uint8_t* buffer, size_t length, uint8_t* hash, size_t *hashLength);
void cx_sha256_init(cx_sha256_t* t);

#endif
