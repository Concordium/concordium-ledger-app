#ifndef _CONCORDIUM_TEST_OS_H_
#define _CONCORDIUM_TEST_OS_H_

#include <stdint.h>  // uint*_t

typedef struct cx_ecfp_private_key_t {
    uint8_t W[33];
} cx_ecfp_private_key_t, cx_ecfp_public_key_t;

int G_io_apdu_buffer[100];

// defining throw to just return the error code, which means throwing in nested functions won't work correctly during testing
#define THROW(error) return(error);
// defining try/catch to do nothing
#define BEGIN_TRY
#define TRY
#define FINALLY
#define END_TRY

// defining unused constants, to be able to compile (These are incorrect)
#define CHANNEL_APDU 1
#define IO_RETURN_AFTER_TX 1
#define HDW_ED25519_SLIP10 1
#define CX_CURVE_Ed25519 1
#define CX_RND_RFC6979 1
#define CX_LAST 1
#define CX_SHA512 1

// redefining U2BE, U4BE
#define U2BE(buf, off) ((((buf)[off]&0xFF)<<8) | ((buf)[off+1]&0xFF) )
#define U4BE(buf, off) ((U2BE(buf, off)<<16) | (U2BE(buf, off+2)&0xFFFF))

#endif
