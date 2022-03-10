#ifndef _CONCORDIUM_APP_CONFIGURE_BAKER_H_
#define _CONCORDIUM_APP_CONFIGURE_BAKER_H_

/**
 * Handles the signing flow for an 'Configure Baker' transaction. It validates
 * that the correct UpdateType is supplied and will fail otherwise.
 * @param cdata please see /doc/ins_configure_delegation.md for details
 */
void handleSignConfigureBaker(
    uint8_t *cdata,
    uint8_t p1,
    uint8_t dataLength,
    volatile unsigned int *flags,
    bool isInitialCall);

// TODO: Consider making a union type between the 3 different
// steps as we will not be using the memory at the same time,
// i.e. displayCapital/displayRestake/displayOpenForDelegation in one part,
// commission rates in their own, and url in its own part.
typedef struct {
    uint8_t displayCapital[26];
    uint8_t displayRestake[4];
    uint8_t displayOpenForDelegation[15];
    uint8_t transactionFeeCommissionRate[43];
    uint8_t bakingRewardCommissionRate[43];
    uint8_t finalizationRewardCommissionRate[43];
    bool hasCapital;
    bool hasRestakeEarnings;
    bool hasOpenForDelegation;
    bool hasKeys;
    bool hasMetadataUrl;
    bool hasTransactionFeeCommission;
    bool hasBakingRewardCommission;
    bool hasFinalizationRewardCommission;
    bool firstDisplay;
    uint16_t urlLength;
    uint8_t url[255];
} signConfigureBaker_t;

#endif
