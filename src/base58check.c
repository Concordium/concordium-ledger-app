/*******************************************************************************
*   Ledger App - Bitcoin Wallet
*   (c) 2016-2019 Ledger
*
*  - Only the 'base58_encode()' method (renamed from btchip_encode_base58).
*    Reformatting of the method has been done, and removal of all PRINTF()
*    invocations, and changed the algorithm to insert a space for every 10 characters.
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include <stdlib.h>
#include "os.h"
#include "cx.h"
#include <string.h>

#define MAX_ENC_INPUT_SIZE 120

unsigned char const BASE58ALPHABET[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

int base58_encode(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen) {
    size_t i = 0, j;
    size_t startAt, stopAt;
    size_t zeroCount = 0;
    size_t outputSize;
    size_t pageSize = 10;

    if (length > MAX_ENC_INPUT_SIZE) {
        return -1;
    }

    while ((zeroCount < length) && (in[zeroCount] == 0)) {
        ++zeroCount;
    }

    outputSize = (length - zeroCount) * 138 / 100 + 1;
    int spaces = outputSize / pageSize;
    outputSize += spaces;
    if (*outlen < outputSize) {
        *outlen = outputSize;
        return -1;
    }

    memset(out, 0, outputSize);
    stopAt = outputSize - 1;
    for (startAt = zeroCount; startAt < length; startAt++) {
        int carry = in[startAt];
        for (j = outputSize - 1; (int)j >= 0; j--) {
            carry += 256 * out[j];
            out[j] = carry % 58;
            carry /= 58;

            if (j <= stopAt - 1 && carry == 0) {
                break;
            }
        }
        stopAt = j;
    }

    j = 0;
    while (j < outputSize && out[j] == 0) {
        j += 1;
    }

    if (*outlen < zeroCount + outputSize - j) {
        *outlen = zeroCount + outputSize - j;
        return -1;
    }

    *outlen = zeroCount + outputSize - j;
    int distance = zeroCount - j;
    size_t nextSpace = pageSize;
    int offset = 0;
    if (distance < 0) {
        for (i = zeroCount; i < *outlen + spaces; ++i) {
            if (i == nextSpace) {
                out[i] = ' ';
                nextSpace += (pageSize + 1);
                offset++;
            } else {
                out[i] = BASE58ALPHABET[out[i - distance - offset]];
            }
        }
    }
    else {
        for (i = *outlen + spaces - 1; (int)i >= 0; --i) {
            if  ((*outlen + spaces - 1 - i) == nextSpace) {
                out[i] = ' ';
                nextSpace += (pageSize + 1);
                offset++;
            } else {
                out[i] = BASE58ALPHABET[out[i - distance + offset]];
            }
        }
    }

    memset(out, BASE58ALPHABET[0], zeroCount);
    return 0;
}

int base58check_encode(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen) {
    // We need room for the version byte, length of input and the first 4 bytes of the SHA256(SHA256(version + in))
    // calculation.
    uint8_t buffer[1 + length + 4];

    // Concordium uses a hardcoded version value of '1', therefore this byte is not received from the computer.
    buffer[0] = 1;

    memmove(&buffer[1], in, length);

    // Calculate SHA256(SHA256(version + in)), and append the first 4 bytes to the (version + in) bytes.
    uint8_t hash[32];
    cx_hash_sha256(buffer, length + 1, hash, sizeof(hash));
    cx_hash_sha256(hash, sizeof(hash), hash, sizeof(hash));
    memmove(&buffer[1 + length], hash, 4);

    // The encoding input is the version + original input + 4 first bytes of double SHA256.
    return base58_encode(buffer, 1 + length + 4, out, outlen);
}
