#include "os.h"
#include "ux.h"
#include "cx.h"
#include <stdbool.h>

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// TODO: The concordium coin type value has to be set to the value we get in SLIP44.
#define CONCORDIUM_COIN_TYPE 691
#define CONCORDIUM_PURPOSE 583

typedef struct {
    uint8_t identity;
    uint8_t accountIndex;
} accountSubtreePath_t;

extern accountSubtreePath_t path;

// Helper object used when computing the hash of a transaction.
typedef struct {
    cx_sha256_t hash;
    uint8_t transactionHash[32];
    bool initialized;
} tx_state_t;

// Each instruction's state has to have its own struct here that is put in the global union below. This translates
// into each handler file having its own struct here.
typedef struct {
    char displayStr[65];
    uint8_t displayAccount[8];
    uint8_t remainingNumberOfScheduledAmounts;
    uint8_t scheduledAmountsInCurrentPacket;

    uint8_t displayAmount[25];
    uint8_t displayTimestamp[25];

    // Buffer to hold the incoming databuffer so that we can iterate over it.
    uint8_t buffer[255];
    uint8_t pos;

    tx_state_t tx_state;
} signTransferContext_t;

// As the Ledger device is very limited on memory, the context of each instruction is stored in a
// shared global union, so that we use no more memory than that of the most space using instruction context.
typedef union {
    signTransferContext_t signTransferContext;
} instructionContext;
extern instructionContext global;

#endif