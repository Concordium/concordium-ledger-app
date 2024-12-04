#ifndef _SIGN_H_
#define _SIGN_H_

#include "ux.h"

#ifdef HAVE_BAGL

extern const ux_flow_step_t ux_sign_flow_shared_review;
extern const ux_flow_step_t ux_sign_flow_shared_sign;
extern const ux_flow_step_t ux_sign_flow_shared_decline;
extern const ux_flow_step_t* const ux_sign_flow_shared[];

#endif

void buildAndSignTransactionHash();

#endif
