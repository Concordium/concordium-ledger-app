#pragma once

int encode_base58(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen);

/**
 * Base58 encodes the input and writes the encoding to the supplied out destination. Returns a
 * non-zero value if the input cannot be validly base58 encoded, i.e. the input is malformed.
 * N.B. The encoding contains a space for every 10th character.
 * An error is thrown if the input length is not exactly 32.
 * @return 0 if input was validly base58 encoded, or -1 if it was not valid base58
 */
int base58check_encode(const unsigned char *in,
                       size_t inlength,
                       unsigned char *out,
                       size_t *outlen);
