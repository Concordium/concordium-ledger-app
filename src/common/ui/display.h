#ifndef _CBOR_H_
#define _CBOR_H_

#include "ux.h"

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
// extern const ux_flow_step_t *const ux_display_memo[];
typedef struct {
    uint32_t cborLength;
    uint32_t displayUsed;
    uint8_t display[255];
    uint8_t majorType;
} cborContext_t;

void handleCborStep(void);

#endif

#ifdef HAVE_BAGL

extern const ux_flow_step_t ux_display_memo_step_nocb;
extern const ux_flow_step_t ux_sign_flow_account_sender_view;

#endif

void uiComparePubkey(void);
void uiGeneratePubkey(volatile unsigned int *flags);
