#ifndef BASE58_CHECK_H
#define BASE58_CHECK_H

int encode_base58(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen);

/**
 * Base58 encodes the input and writes the encoding to the supplied out destination. Returns a 
 * non-zero value if the input cannot be validly base58 encoded, i.e. the input is malformed.
 */
int base58check_encode(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen);

#endif
