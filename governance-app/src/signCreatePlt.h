#ifndef _CONCORDIUM_APP_CREATE_PLT_H_
#define _CONCORDIUM_APP_CREATE_PLT_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Handles the signing flow, including updating the display, for the 'create PLT'
 * update instruction.
 * @param cdata please see /doc/ins_create_plt.md for details
 */
void handleSignCreatePlt(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall);

typedef enum {
    TX_CREATE_PLT_INITIAL = 70,
    TX_CREATE_PLT_PAYLOAD = 71,
    TX_CREATE_PLT_INIT_PARAMS = 72,
} createPltState_t;

typedef struct {
    // state
    uint32_t initializationParamsLength;
    uint32_t remainingInitializationParamsBytes;

    // Display fields
    char updateTypeText[32];
    uint8_t tokenId[129];      // Max 128 chars + null terminator
    char tokenModule[68];      // Hex representation (32 bytes * 2 + 4 chars for pagination + null)
    uint8_t decimals[4];       // String representation of uint8
    uint8_t initParams[512];   // Buffer to store init params data (up to 512 bytes)
    char initParamsHex[1089];  // Hex representation for display (512 bytes * 2 + (512/8=64) chars for pagination + null)

    createPltState_t state;
} signCreatePltContext_t;

#endif
