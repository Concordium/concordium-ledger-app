#pragma once

#define INS_VERIFY_ADDRESS 0x00

// An INS instruction containing 0x01 means that we should start the public-key flow.
#define INS_GET_PUBLIC_KEY 0x01

// An INS instruction containing 0x02 means that we should start the transfer signing flow.
#define INS_SIGN_TRANSFER 0x02

// An INS instruction containing 0x03 means that we should start the scheduled transfer signing
// flow.
#define INS_SIGN_TRANSFER_WITH_SCHEDULE 0x03

// An INS instruction containing 0x04 means that we should start the credential deployment signing
// flow.
#define INS_CREDENTIAL_DEPLOYMENT 0x04

#define INS_EXPORT_PRIVATE_KEY 0x05

#define INS_DEPLOY_MODULE 0x06

#define INS_INIT_CONTRACT 0x07

#define INS_UPDATE_CONTRACT    0x08
#define INS_TRANSFER_TO_PUBLIC 0x12

#define INS_CONFIGURE_DELEGATION 0x17
#define INS_CONFIGURE_BAKER      0x18

#define INS_PUBLIC_INFO_FOR_IP 0x20

#define INS_SIGN_UPDATE_CREDENTIAL 0x31

#define INS_SIGN_TRANSFER_WITH_MEMO              0x32
#define INS_SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO 0x34
#define INS_REGISTER_DATA                        0x35

#define INS_APP_NAME     0x36
#define INS_GET_APP_NAME 0x21

int handler(uint8_t INS,
            uint8_t *cdata,
            uint8_t p1,
            uint8_t p2,
            uint8_t lc,
            volatile unsigned int *flags,
            bool isInitialCall);
