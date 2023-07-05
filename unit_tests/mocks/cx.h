#ifndef _CONCORDIUM_TEST_CX_H_
#define _CONCORDIUM_TEST_CX_H_

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

void* cx_hash_sha256(uint8_t* buffer, size_t length, uint8_t* hash, size_t hashLength);

typedef struct cx_sha256_s {
    size_t dummy;
} cx_sha256_t, cx_hash_t;

void hash(
    cx_hash_t* hash,
    uint32_t mode,
    const unsigned char* in,
    unsigned int len,
    unsigned char* out,
    unsigned int out_len);
typedef uint32_t cx_err_t;
cx_err_t
cx_hash_no_throw(cx_hash_t* ctx, uint8_t mode, uint8_t* buffer, size_t length, uint8_t* hash, size_t* hashLength);

void cx_sha256_init(cx_sha256_t* t);

// From "lcx_hash.h"
enum cx_md_e {
    CX_NONE = 0,    ///< No message digest algorithm
    CX_SHA256 = 3,  ///< SHA256 digest
    CX_SHA512 = 5,  ///< SHA512 digest
};
typedef enum cx_md_e cx_md_t;

#define CX_OK 0x00000000

#endif
