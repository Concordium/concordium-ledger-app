#include "os.h"
#include "ux.h"

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// TODO: The concordium coin type value has to be set to the value we get in SLIP44.
#define CONCORDIUM_COIN_TYPE 691

typedef struct {
    uint8_t identity;
    uint8_t accountIndex;
} accountSubtreePath_t;

extern accountSubtreePath_t path;

#endif