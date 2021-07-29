#ifndef _CONCORDIUM_APP_ADD_IDENTITY_PROVIDER_H_
#define _CONCORDIUM_APP_ADD_IDENTITY_PROVIDER_H_

/**
 * Handles the signing flow, including updating the display, for the 'add identity provider'
 * update instruction.
 * @param cdata please see /doc/ins_add_identity_provider.md for details
 */
void handleSignAddIdentityProvider(uint8_t *cdata, uint8_t p1, uint8_t dataLength, volatile unsigned int *flags, bool isInitialCall);

typedef enum {
              TX_ADD_IDENTITY_PROVIDER_INITIAL = 35,
              TX_ADD_IDENTITY_PROVIDER_DESCRIPTION_LENGTH = 36,
              TX_ADD_IDENTITY_PROVIDER_DESCRIPTION = 37,
              TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY_LENGTH = 38,
              TX_ADD_IDENTITY_PROVIDER_VERIFY_KEY = 39,
              TX_ADD_IDENTITY_PROVIDER_CDI_VERIFY_KEY = 39
} addIdentityProviderState_t;

typedef struct {
  uint32_t descriptionLength;
  uint32_t verifyKeyLength;
  uint8_t ipIdentity[4];
  char cdiVerifyKey[64];
  addIdentityProviderState_t state;
} signAddIdentityProviderContext_t;

#endif
