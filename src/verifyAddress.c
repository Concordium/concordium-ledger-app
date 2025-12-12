#include "globals.h"

#define LEGACY_ACCOUNT_SUBTREE 0
#define LEGACY_NORMAL_ACCOUNTS 0

static verifyAddressContext_t *ctx = &global.verifyAddressContext;

static const uint32_t HARDENED_OFFSET = 0x80000000;

// gX and gY are the coordinates of g, which is the first part of the onchainCommitmentKey.
static const uint8_t gX[48] = {
    0x11, 0x4c, 0xbf, 0xe4, 0x4a, 0x02, 0xc6, 0xb1, 0xf7, 0x87, 0x11, 0x17, 0x6d, 0x5f, 0x43, 0x72,
    0x95, 0x36, 0x7a, 0xa4, 0xf2, 0xa8, 0xc2, 0x55, 0x1e, 0xe1, 0x0d, 0x25, 0xa0, 0x3a, 0xdc, 0x69,
    0xd6, 0x1a, 0x33, 0x2a, 0x05, 0x89, 0x71, 0x91, 0x9d, 0xad, 0x73, 0x12, 0xe1, 0xfc, 0x94, 0xc5};
static const uint8_t gY[48] = {
    0x18, 0x6a, 0xf3, 0x21, 0x19, 0x54, 0x39, 0x13, 0xb2, 0x6a, 0x46, 0x2a, 0x02, 0x31, 0xe4, 0xbf,
    0x5f, 0xde, 0xe0, 0xb5, 0x2c, 0x91, 0x6f, 0x68, 0x85, 0x44, 0x87, 0xe8, 0x11, 0x2c, 0x1f, 0x27,
    0x74, 0x35, 0xfc, 0x07, 0x6f, 0x3a, 0xda, 0xd5, 0x6d, 0x18, 0xd8, 0x6a, 0x65, 0x99, 0xb5, 0x42};

/*
 * Calculates the credId from the given prf key and credential counter.
 * The size of the computed credId is 48 bytes.
 */
cx_err_t getCredId(uint8_t *prf,
                   size_t prfSize,
                   uint32_t credCounter,
                   uint8_t *credId,
                   size_t credIdSize) {
    cx_err_t error = 0;

    // get bn lock to allow working with binary numbers and elliptic curves
    error = cx_bn_lock(16, 0);
    if (error != 0) {
        return error;
    }

    // Initialize binary numbers
    cx_bn_t credIdExponentBn, tmpBn, rBn, ccBn, prfBn;
    CX_CHECK(cx_bn_alloc(&credIdExponentBn, 32));
    CX_CHECK(cx_bn_alloc(&tmpBn, 32));
    CX_CHECK(cx_bn_alloc_init(&prfBn, 32, prf, prfSize));
    CX_CHECK(cx_bn_alloc_init(&rBn, 32, bls12_381_r, sizeof(bls12_381_r)));
    CX_CHECK(cx_bn_alloc(&ccBn, 32));
    CX_CHECK(cx_bn_set_u32(ccBn, credCounter));

    // Apply cred counter offset
    CX_CHECK(cx_bn_mod_add(tmpBn, prfBn, ccBn, rBn));

    // Inverse of (prf + cred_counter) is the exponent for calculating the credId
    CX_CHECK(cx_bn_mod_invert_nprime(credIdExponentBn, tmpBn, rBn));

    // clean up binary numbers
    CX_CHECK(cx_bn_destroy(&tmpBn));
    CX_CHECK(cx_bn_destroy(&rBn));
    CX_CHECK(cx_bn_destroy(&prfBn));
    CX_CHECK(cx_bn_destroy(&ccBn));

    // initialize elliptic curve point given by global commitmentKey
    cx_ecpoint_t commitmentKey;
    CX_CHECK(cx_ecpoint_alloc(&commitmentKey, CX_CURVE_BLS12_381_G1));
    CX_CHECK(cx_ecpoint_init(&commitmentKey, gX, sizeof(gX), gY, sizeof(gY)));

    //  multiply commitmentKey with credIdExponent
    CX_CHECK(cx_ecpoint_scalarmul_bn(&commitmentKey, credIdExponentBn));
    CX_CHECK(cx_bn_destroy(&credIdExponentBn));

    // calculate credId which is the compressed version of commitmentKey * credIdExponent
    cx_bn_t x, y, negy;
    CX_CHECK(cx_bn_alloc(&x, 48));
    CX_CHECK(cx_bn_alloc(&y, 48));
    CX_CHECK(cx_bn_alloc(&negy, 48));

    CX_CHECK(cx_ecpoint_export_bn(&commitmentKey, &x, &y));
    CX_CHECK(cx_bn_export(x, credId, credIdSize));

    // Calculate negation of the point to get -y
    CX_CHECK(cx_ecpoint_neg(&commitmentKey));
    CX_CHECK(cx_ecpoint_export_bn(&commitmentKey, &x, &negy));

    int diff;
    CX_CHECK(cx_bn_cmp(y, negy, &diff));

    // cleanup binary numbers
    CX_CHECK(cx_bn_destroy(&x));
    CX_CHECK(cx_bn_destroy(&y));
    CX_CHECK(cx_bn_destroy(&negy));
    CX_CHECK(cx_ecpoint_destroy(&commitmentKey));

    credId[0] |= 0x80;  // Indicate this is on compressed form
    if (diff > 0) {
        credId[0] |= 0x20;  // Indicate that y > -y
    }

    // CX_CHECK label to goto in case of an error
end:
    cx_bn_unlock();
    return error;
}

void handleVerifyAddress(uint8_t *cdata, uint8_t p1, uint8_t lc, volatile unsigned int *flags) {
    size_t offset = 0;
    bool is_new_path = p1 == 0x01;
    uint32_t identityProvider = 0;
    uint8_t remainingDataLength = lc;
    if (is_new_path) {
        if (remainingDataLength < 4) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        identityProvider = U4BE(cdata, offset);
        offset += 4;
        remainingDataLength -= 4;
    }
    if (remainingDataLength < 4) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    uint32_t identity = U4BE(cdata, offset);
    offset += 4;
    remainingDataLength -= 4;
    if (remainingDataLength < 4) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    uint32_t credCounter = U4BE(cdata, offset);

    size_t prfKeyPathLen = is_new_path ? 5 : 6;
    uint32_t *prfKeyPath;
    if (is_new_path) {
        prfKeyPath = (uint32_t[5]){NEW_PURPOSE | HARDENED_OFFSET,
                                   NEW_COIN_TYPE | HARDENED_OFFSET,
                                   identityProvider | HARDENED_OFFSET,
                                   identity | HARDENED_OFFSET,
                                   NEW_PRF_KEY | HARDENED_OFFSET};
    } else {
        prfKeyPath = (uint32_t[6]){LEGACY_PURPOSE | HARDENED_OFFSET,
                                   LEGACY_COIN_TYPE | HARDENED_OFFSET,
                                   LEGACY_ACCOUNT_SUBTREE | HARDENED_OFFSET,
                                   LEGACY_NORMAL_ACCOUNTS | HARDENED_OFFSET,
                                   identity | HARDENED_OFFSET,
                                   LEGACY_PRF_KEY | HARDENED_OFFSET};
    }

    if (is_new_path) {
        getIdentityAccountDisplayNewPath(ctx->display,
                                         sizeof(ctx->display),
                                         identityProvider,
                                         identity,
                                         credCounter);
    } else {
        getIdentityAccountDisplay(ctx->display, sizeof(ctx->display), identity, credCounter);
    }

    uint8_t credId[48];
    uint8_t prf[32];
    BEGIN_TRY {
        TRY {
            getBlsPrivateKey(prfKeyPath, prfKeyPathLen, prf, sizeof(prf));
            cx_err_t error = getCredId(prf, sizeof(prf), credCounter, credId, sizeof(credId));
            if (error != 0) {
                THROW(ERROR_INVALID_STATE);
            }
        }
        FINALLY {
            explicit_bzero(prf, sizeof(prf));
        }
    }
    END_TRY;

    uint8_t accountAddress[32];
    cx_err_t error = 0;
    error = cx_hash_sha256(credId, sizeof(credId), accountAddress, sizeof(accountAddress));
    if (error == 0) {
        THROW(ERROR_FAILED_CX_OPERATION);
    }
    size_t addressLength = sizeof(ctx->address);

    base58check_encode(accountAddress, sizeof(accountAddress), ctx->address, &addressLength);
    ctx->address[55] = '\0';

    uiVerifyAddress(flags);
}
