#ifndef _SIGN_H_
#define _SIGN_H_

extern const ux_flow_step_t* const ux_sign_flow_shared[];
void buildAndSignTransactionHash();
void declineToSignTransaction();

#endif