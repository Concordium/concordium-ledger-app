#ifndef _MEMO_H_
#define _MEMO_H_

void readMemoInitial(uint8_t *cdata, uint8_t dataLength);
void readMemoContent(uint8_t *cdata, uint8_t dataLength);
extern const ux_flow_step_t* const ux_sign_transfer_memo[];


typedef struct {
    uint32_t memoLength;
    uint8_t memo[255];
    uint8_t majorType;
} memoContext_t;

#endif
