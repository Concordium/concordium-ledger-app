#ifndef _MEMO_H_
#define _MEMO_H_

void readMemoInitial(uint8_t *cdata, uint8_t dataLength);
void readMemoContent(uint8_t *cdata, uint8_t dataLength);
void displayMemo(volatile unsigned int *flags);

typedef struct {
    uint32_t memoLength;
    uint8_t memo[255];
    uint8_t majorType;
} memoContext_t;

#endif
