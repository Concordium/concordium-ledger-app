#ifndef _CONCORDIUM_TEST_CX_H_
#define _CONCORDIUM_TEST_CX_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

void* cx_hash_sha256(uint8_t* buffer, size_t length, uint8_t* hash, size_t hashLength);

typedef struct cx_sha256_s {
    size_t dummy;
} cx_sha256_t, cx_hash_t;

void cx_hash(cx_hash_t* ctx, uint8_t mode, uint8_t* buffer, size_t length, uint8_t* hash, size_t* hashLength);
void cx_sha256_init(cx_sha256_t* t);

// From "lcx_hash.h"
enum cx_md_e {
    CX_NONE = 0,    ///< No message digest algorithm
    CX_SHA256 = 3,  ///< SHA256 digest
    CX_SHA512 = 5,  ///< SHA512 digest
};
typedef enum cx_md_e cx_md_t;

#endif
