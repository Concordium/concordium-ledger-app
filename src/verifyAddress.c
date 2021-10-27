/*
   hmac_sha256.c
   Originally written by https://github.com/h5p9sl
 */

#include <os.h>
#include "cx.h"
#include "ux.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "globals.h"
#include "base58check.h"
#include "responseCodes.h"

#define ACCOUNT_SUBTREE 0
#define NORMAL_ACCOUNTS 0

#define l_CONST 48
#define OKM_SPLIT 31
#define BYTE_LENGTH 48

static verifyAddressContext_t *ctx = &global.verifyAddressContext;
static keyDerivationPath_t *keyPath = &path;

static const uint32_t HARDENED_OFFSET = 0x80000000;
static const uint8_t r[32] = {0x73, 0xed, 0xa7, 0x53, 0x29, 0x9d, 0x7d, 0x48, 0x33, 0x39, 0xd8, 0x08, 0x09, 0xa1, 0xd8, 0x05, 0x53, 0xbd, 0xa4, 0x02, 0xff, 0xfe, 0x5b, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01};
static const uint8_t saltSeed[20] = {66, 76, 83, 45, 83, 73, 71, 45, 75, 69, 89, 71, 69, 78, 45, 83, 65, 76, 84, 45};
static const uint8_t l_bytes[2] = {l_CONST, 0};

static const uint8_t gX[48] = {0x11, 0x4c, 0xbf, 0xe4, 0x4a, 0x02, 0xc6, 0xb1, 0xf7, 0x87, 0x11, 0x17, 0x6d, 0x5f, 0x43, 0x72, 0x95, 0x36, 0x7a, 0xa4, 0xf2, 0xa8, 0xc2, 0x55, 0x1e, 0xe1, 0x0d, 0x25, 0xa0, 0x3a, 0xdc, 0x69, 0xd6, 0x1a, 0x33, 0x2a, 0x05, 0x89, 0x71, 0x91, 0x9d, 0xad, 0x73, 0x12, 0xe1, 0xfc, 0x94, 0xc5};
static const uint8_t gY[48] = {0x18, 0x6a, 0xf3, 0x21, 0x19, 0x54, 0x39, 0x13, 0xb2, 0x6a, 0x46, 0x2a, 0x02, 0x31, 0xe4, 0xbf, 0x5f, 0xde, 0xe0, 0xb5, 0x2c, 0x91, 0x6f, 0x68, 0x85, 0x44, 0x87, 0xe8, 0x11, 0x2c, 0x1f, 0x27, 0x74, 0x35, 0xfc, 0x07, 0x6f, 0x3a, 0xda, 0xd5, 0x6d, 0x18, 0xd8, 0x6a, 0x65, 0x99, 0xb5, 0x42};


UX_STEP_NOCB(
    ux_verify_address_0_step,
    bnnn_paging,
    {
        .title = "Verify Address",
        .text = (char *) global.verifyAddressContext.display
    });

UX_STEP_CB(
    ux_verify_address_1_step,
    bnnn_paging,
    sendSuccess(0),
    {
        .title = "Address",
        .text = (char *) global.verifyAddressContext.address
    });
UX_FLOW(ux_verify_address,
        &ux_verify_address_0_step,
        &ux_verify_address_1_step
    );


// dst should have length 32;
void blsKeygen(const uint8_t *seed, size_t seedLength, uint8_t *dst, size_t dstLength) {
    uint8_t okm[l_CONST] = {0};
    uint8_t sk[l_CONST];
    uint8_t h[32];
    uint8_t salt[32];
    uint8_t ikm[seedLength + 1];
    memcpy(ikm, seed, seedLength);
    ikm[seedLength] = 0;

    cx_hash_sha256(saltSeed, sizeof(saltSeed), salt, sizeof(salt));
    while (cx_math_is_zero(okm, sizeof(okm))) {
        cx_hkdf_extract(CX_SHA256, ikm, sizeof(ikm), salt, sizeof(salt), h);
        cx_hkdf_expand(CX_SHA256, h, sizeof(h), l_bytes, sizeof(l_bytes), okm, sizeof(okm));

        // Switch from big-endian to small-endian
        for (uint8_t i = 0; i < l_CONST; i++) {
            sk[i] = okm[l_CONST - i - 1];
        }

        cx_math_modm(sk, sizeof(sk), r, sizeof(r));
        cx_hash_sha256(salt, sizeof(salt), salt, sizeof(salt));
    }
    memmove(dst, sk + l_CONST - dstLength, dstLength);
}

/* prf and credId should atleast have length 32 */
cx_err_t getCredId(uint8_t *prf, size_t prfSize, uint32_t cred_counter, uint8_t *credId, size_t credIdSize) {
    cx_err_t error = 0;

    cx_bn_lock(16, 0);
    cx_bn_t prf_exp, tmp_bn, r_bn, cc_bn, prf_bn;
    CX_CHECK(cx_bn_alloc(&prf_exp, 32));
    CX_CHECK(cx_bn_alloc(&cc_bn, 32));
    CX_CHECK(cx_bn_alloc(&tmp_bn, 32));
    CX_CHECK(cx_bn_alloc_init(&prf_bn, 32, prf, prfSize));
    CX_CHECK(cx_bn_alloc_init(&r_bn, 32, r, sizeof(r)));

    CX_CHECK(cx_bn_set_u32(cc_bn, cred_counter));
    CX_CHECK(cx_bn_mod_add(tmp_bn, prf_bn, cc_bn, r_bn));

    CX_CHECK(cx_bn_mod_invert_nprime(prf_exp, tmp_bn, r_bn));

    CX_CHECK(cx_bn_destroy(&tmp_bn));
    CX_CHECK(cx_bn_destroy(&r_bn));
    CX_CHECK(cx_bn_destroy(&prf_bn));
    CX_CHECK(cx_bn_destroy(&cc_bn));

    uint32_t sign = 0;

    cx_ecpoint_t commitmentKey;
    CX_CHECK(cx_ecpoint_alloc(&commitmentKey, CX_CURVE_BLS12_381_G1));
    CX_CHECK(cx_ecpoint_init(&commitmentKey, gX, sizeof(gX), gY, sizeof(gY)));
    CX_CHECK(cx_ecpoint_scalarmul_bn(&commitmentKey, prf_exp));
    CX_CHECK(cx_bn_destroy(&prf_exp));

    CX_CHECK(cx_ecpoint_compress(&commitmentKey, credId, credIdSize, &sign));

    credId[0] += 0x80; // Indicate this is on compressed form
    if (sign == 1) {
        credId[0] += 0x20; // Indicate that y is the larger of the two possibilities
    }

end:
    cx_bn_unlock();
    return error;
}

/**
 * Change the interpretation of the given byte array as a utf8 string. The output should have double the size of the input.
 * N.B. Added to duplicate error in Concordium desktop wallet.
 */
void InterpretHexAsUtf8(uint8_t *byteArray, const uint64_t len, uint8_t *transformed, const size_t transformedSize) {
    static uint8_t const hex[] = "0123456789abcdef";

    if (transformedSize < len * 2) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    for (uint64_t i = 0; i < len; i++) {
        transformed[2 * i] = hex[(byteArray[i]>>4) & 0x0F];
        transformed[2 * i + 1] = hex[(byteArray[i]>>0) & 0x0F];
    }
}

void handleVerifyAddress(uint8_t *cdata, volatile unsigned int *flags) {
    parseKeyDerivationPath(cdata);
    uint32_t identity = keyPath->rawKeyDerivationPath[4];
    uint32_t cred_counter = keyPath->rawKeyDerivationPath[6];

    getIdentityAccountDisplay(ctx->display, sizeof(ctx->display));

    uint32_t keyDerivationPath[6] = {
        CONCORDIUM_PURPOSE | HARDENED_OFFSET,
        CONCORDIUM_COIN_TYPE | HARDENED_OFFSET,
        ACCOUNT_SUBTREE | HARDENED_OFFSET,
        NORMAL_ACCOUNTS | HARDENED_OFFSET,
        identity | HARDENED_OFFSET,
        1 | HARDENED_OFFSET
    };

    cx_ecfp_private_key_t seed_t;
    getPrivateKey(keyDerivationPath, 6, &seed_t);

    uint8_t seed[64] = {0};
    InterpretHexAsUtf8(seed_t.d, sizeof(seed_t.d), seed, sizeof(seed));

    uint8_t prf[32] = {0};
    blsKeygen(seed, sizeof(seed), prf, sizeof(prf));

    uint8_t credId[48] = {0};
    cx_err_t error = getCredId(prf, sizeof(prf), cred_counter, credId, sizeof(credId));

    if (error != 0) {
        THROW(ERROR_INVALID_STATE);
    }

    uint8_t accountAddress[32] = {0};
    cx_hash_sha256(credId, sizeof(credId), accountAddress, sizeof(accountAddress));

    size_t addressLength = sizeof(ctx->address);

    base58check_encode(accountAddress, sizeof(accountAddress), ctx->address, &addressLength);
    ctx->address[55] = '\0';

    ux_flow_init(0, ux_verify_address, NULL);
    *flags |= IO_ASYNCH_REPLY;
}
