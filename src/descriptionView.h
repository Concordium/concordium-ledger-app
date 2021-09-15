#ifndef _DESCRIPTION_VIEW_H_
#define _DESCRIPTION_VIEW_H_

void displayDescriptionPart(volatile unsigned int *flags);
void handleDescriptionPart(void);

typedef enum {
    DESC_NAME,
    DESC_URL,
    DESC_DESCRIPTION
} descriptionState_t;

typedef struct {
    uint32_t textLength;
    uint8_t header[12];
    uint8_t text[255];
    descriptionState_t descriptionState;
} descriptionContext_t;

#endif
