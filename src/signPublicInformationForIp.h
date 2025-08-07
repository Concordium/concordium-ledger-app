#pragma once

#import "buffer.h"

/**
 * Handles the signing flow, including updating the display, for the signing
 * of the public information for the identity provider.
 *
 * @param cdata please see /doc/ins_public_info_for_ip.md for details
 */
void handleSignPublicInformationForIp(uint8_t *cdata,
                                      uint8_t p1,
                                      uint8_t lc,
                                      volatile unsigned int *flags,
                                      bool isInitialCall);

typedef enum {
    TX_PUBLIC_INFO_FOR_IP_INITIAL = 22,
    TX_PUBLIC_INFO_FOR_IP_VERIFICATION_KEY = 23,
    TX_PUBLIC_INFO_FOR_IP_THRESHOLD = 24
} publicInfoForIpState_t;

typedef struct {
    bool showIntro;
    uint8_t publicKeysLength;
    char publicKey[68];
    uint8_t threshold[4];
    char idCredPub[48 * 2 + 1];
    char credId[48 * 2 + 1];

    char keyType[2 + 1];
    publicInfoForIpState_t state;
} signPublicInformationForIp_t;
