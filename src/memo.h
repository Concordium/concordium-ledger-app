#ifndef _MEMO_H_
#define _MEMO_H_

/**
 * Read a CBOR encoded memo's initial part, i.e. the header, which contains the major type and length
 * Only supports major type 0, 1 and 3 (non-negative integers, negative integers and utf-8 strings)
 * Does not streaming (shortCount = 31).
 */
void readMemoInitial(uint8_t *cdata, uint8_t dataLength);
void readMemoContent(uint8_t *cdata, uint8_t dataLength);
extern const ux_flow_step_t* const ux_sign_transfer_memo[];


typedef struct {
    uint32_t memoLength;
    uint32_t memoDisplayUsed;
    uint8_t memo[255];
    uint8_t majorType;
} memoContext_t;

#endif
