#ifndef _DESCRIPTION_VIEW_H_
#define _DESCRIPTION_VIEW_H_

extern const ux_flow_step_t* const ux_sign_description_name[];
extern const ux_flow_step_t* const ux_sign_description_url[];
extern const ux_flow_step_t* const ux_sign_description_description[];

typedef enum {
    NAME,
    URL,
    DESCRIPTION
} descriptionState_t;

typedef struct {
    uint32_t textLength;
    uint8_t text[255];
    descriptionState_t descriptionState;
} descriptionContext_t;

#endif
