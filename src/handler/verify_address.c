#include "verify_address.h"
#include <cx.h>
#include <os.h>

#include "../globals.h"
#include "../types.h"
#include "../sw.h"
#include "../helper/util.h"
#include "../constants.h"
#include "../ui/display.h" // For ui_display_verify_address


// gX and gY are the coordinates of g, which is the first part of the onchainCommitmentKey.
static const uint8_t gX[48] = {0x11, 0x4c, 0xbf, 0xe4, 0x4a, 0x02, 0xc6, 0xb1, 0xf7, 0x87, 0x11, 0x17,
                               0x6d, 0x5f, 0x43, 0x72, 0x95, 0x36, 0x7a, 0xa4, 0xf2, 0xa8, 0xc2, 0x55,
                               0x1e, 0xe1, 0x0d, 0x25, 0xa0, 0x3a, 0xdc, 0x69, 0xd6, 0x1a, 0x33, 0x2a,
                               0x05, 0x89, 0x71, 0x91, 0x9d, 0xad, 0x73, 0x12, 0xe1, 0xfc, 0x94, 0xc5};
static const uint8_t gY[48] = {0x18, 0x6a, 0xf3, 0x21, 0x19, 0x54, 0x39, 0x13, 0xb2, 0x6a, 0x46, 0x2a,
                               0x02, 0x31, 0xe4, 0xbf, 0x5f, 0xde, 0xe0, 0xb5, 0x2c, 0x91, 0x6f, 0x68,
                               0x85, 0x44, 0x87, 0xe8, 0x11, 0x2c, 0x1f, 0x27, 0x74, 0x35, 0xfc, 0x07,
                               0x6f, 0x3a, 0xda, 0xd5, 0x6d, 0x18, 0xd8, 0x6a, 0x65, 0x99, 0xb5, 0x42};


// The methods below are coded to work for the legacy address format.

cx_err_t get_credential_id(uint8_t *prf_key, size_t prf_key_len, uint32_t credential_counter, uint8_t *credential_id, size_t credential_id_len) {
    cx_err_t error = 0;


    // get bn lock to allow working with binary numbers and elliptic curves
    CX_CHECK(cx_bn_lock(16, 0));
    // Initialize binary numbers
    cx_bn_t credIdExponentBn, tmpBn, rBn, ccBn, prfBn;
    CX_CHECK(cx_bn_alloc(&credIdExponentBn, 32));
    CX_CHECK(cx_bn_alloc(&tmpBn, 32));
    CX_CHECK(cx_bn_alloc_init(&prfBn, 32, prf_key, prf_key_len));
    CX_CHECK(cx_bn_alloc_init(&rBn, 32, r, sizeof(r)));
    CX_CHECK(cx_bn_alloc(&ccBn, 32));
    CX_CHECK(cx_bn_set_u32(ccBn, credential_counter));

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
    CX_CHECK(cx_bn_export(x, credential_id, credential_id_len));

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

    credential_id[0] |= 0x80;  // Indicate this is on compressed form
    if (diff > 0) {
        credential_id[0] |= 0x20;  // Indicate that y > -y
    }

    // CX_CHECK label to goto in case of an error
end:
    if(error != CX_OK) {
        PRINTF("something went wrong\n");
    }
    cx_bn_unlock();
    return error;
}

int handler_verify_address(buffer_t *cdata, bool is_new_address) {
    // Reset context
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;

    if(cdata->size != 8 && cdata->size != 12) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }
    // Set the idp index to 0xffffffff
    G_context.verify_address_info.idp_index = 0xffffffff;
    // Read the idp index if it is a new address
    if(is_new_address) {
        if(!buffer_read_u32(cdata, &G_context.verify_address_info.idp_index, BE)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }
    }

    // Read the identity index and credential counter
    if (
        !buffer_read_u32(cdata, &G_context.verify_address_info.identity_index, BE) ||
        !buffer_read_u32(cdata, &G_context.verify_address_info.credential_counter, BE)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    size_t prf_key_path_len = is_new_address ? 5 : 6;
    uint32_t *prf_key_path;
    if(is_new_address) {
        prf_key_path = (uint32_t[5]) {
            NEW_PURPOSE | HARDENED_OFFSET,
            NEW_COIN_TYPE | HARDENED_OFFSET,
            G_context.verify_address_info.idp_index | HARDENED_OFFSET,
            G_context.verify_address_info.identity_index | HARDENED_OFFSET,
            NEW_PRF_KEY | HARDENED_OFFSET
        };
    }
    else{
        prf_key_path = (uint32_t[6]) {
            LEGACY_PURPOSE | HARDENED_OFFSET,
            LEGACY_COIN_TYPE | HARDENED_OFFSET,
            LEGACY_ACCOUNT_SUBTREE | HARDENED_OFFSET,
            LEGACY_NORMAL_ACCOUNT | HARDENED_OFFSET,
            G_context.verify_address_info.identity_index | HARDENED_OFFSET,
            LEGACY_PRF_KEY | HARDENED_OFFSET
        };
    }


    uint8_t credential_id[CREDENTIAL_ID_LEN];
    uint8_t prf_key[32];

    if(get_bls_private_key(prf_key_path, prf_key_path_len, prf_key, sizeof(prf_key)) == -1) {
        return io_send_sw(SW_VERIFY_ADDRESS_FAIL);
    }

    if(get_credential_id(prf_key, 
                        sizeof(prf_key), 
                        G_context.verify_address_info.credential_counter, 
                        credential_id, 
                        sizeof(credential_id)) != CX_OK) {
        return io_send_sw(SW_VERIFY_ADDRESS_FAIL)   ;
    }

    // Empty prf_key
    explicit_bzero(prf_key, sizeof(prf_key));

    uint8_t account_address[32];
    cx_hash_sha256(credential_id, sizeof(credential_id), account_address, sizeof(account_address));

    size_t address_len = sizeof(G_context.verify_address_info.address);

    // This function will return the number of bytes encoded, or -1 on error.
    int rtn = address_to_base58(account_address, sizeof(account_address), G_context.verify_address_info.address, address_len);
    if(rtn < 0) {
        return io_send_sw(SW_VERIFY_ADDRESS_FAIL);
    }

    return ui_display_verify_address();

}
