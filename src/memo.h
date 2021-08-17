#ifndef _MEMO_H_
#define _MEMO_H_

extern const ux_flow_step_t* const ux_sign_transfer_memo[];

void readMemoInitial(uint8_t *cdata, uint8_t dataLength);
void readMemoContent(uint8_t *cdata, uint8_t dataLength);

typedef struct {
    uint32_t memoLength;
    uint8_t memo[255];
    uint8_t majorType;
} memoContext_t;

#endif
