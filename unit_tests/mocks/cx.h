#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t

void * cx_hash_sha256 (uint8_t* buffer, size_t length, uint8_t* hash, size_t *hashLength);
