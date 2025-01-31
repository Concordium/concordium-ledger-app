#pragma once

#include "globals.h"

#ifdef HAVE_BAGL

extern const ux_flow_step_t ux_sign_flow_shared_review;
extern const ux_flow_step_t ux_sign_flow_shared_sign;
extern const ux_flow_step_t ux_sign_flow_shared_decline;
extern const ux_flow_step_t *const ux_sign_flow_shared[];

#endif

/**
 * Read a CBOR encoded dataBlob's initial part, i.e. the header, which contains the major type and
 * length Only supports major type 0, 1 and 3 (non-negative integers, negative integers and utf-8
 * strings) Does not support streaming (shortCount = 31).
 */
void readCborInitial(uint8_t *cdata, uint8_t dataLength);
/**
 * Read part of a CBOR encoded dataBlob. Should only be used after readCborInitial.
 */
void readCborContent(uint8_t *cdata, uint8_t dataLength);

void buildAndSignTransactionHash();
