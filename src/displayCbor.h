#ifndef _CBOR_H_
#define _CBOR_H_

#include "ux.h"

/**
 * Read a CBOR encoded dataBlob's initial part, i.e. the header, which contains the major type and length
 * Only supports major type 0, 1 and 3 (non-negative integers, negative integers and utf-8 strings)
 * Does not streaming (shortCount = 31).
 */
void readCborInitial(uint8_t *cdata, uint8_t dataLength);
void readCborContent(uint8_t *cdata, uint8_t dataLength);
extern const ux_flow_step_t *const ux_display_cbor[];

typedef struct {
    uint32_t cborLength;
    uint32_t displayUsed;
    uint8_t display[255];
    uint8_t majorType;
} CborContext_t;

#endif
