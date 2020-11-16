#ifndef BASE58_H
#define BASE58_H

int encode_base58(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen);
int base58check_encode(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen);

#endif
