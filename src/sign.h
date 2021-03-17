#ifndef _SIGN_H_
#define _SIGN_H_

extern const ux_flow_step_t ux_sign_flow_shared_review;
extern const ux_flow_step_t ux_sign_flow_shared_sign;
extern const ux_flow_step_t ux_sign_flow_shared_decline;
extern const ux_flow_step_t* const ux_sign_flow_shared[];

void buildAndSignTransactionHash();
void declineToSignTransaction();

#endif