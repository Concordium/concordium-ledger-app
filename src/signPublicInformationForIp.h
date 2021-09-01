#ifndef _CONCORDIUM_APP_PUBLIC_INFO_FOR_IP_H_
#define _CONCORDIUM_APP_PUBLIC_INFO_FOR_IP_H_

/**
 * Handles the signing flow, including updating the display, for the signing 
 * of the public information for the identity provider.
 * 
 * @param cdata please see /doc/ins_public_info_for_ip.md for details
 */
void handleSignPublicInformationForIp(uint8_t *cdata, uint8_t p1, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
    TX_PUBLIC_INFO_FOR_IP_INITIAL = 22,
    TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY = 23,
    TX_PUBLIC_INFO_FOR_IP_THRESHOLD = 24
} publicInfoForIpState_t;

typedef struct {
    uint8_t publicKeysLength;
    char publicKey[65];
    uint8_t threshold[4];
    publicInfoForIpState_t state;
} signPublicInformationForIp_t;

#endif
