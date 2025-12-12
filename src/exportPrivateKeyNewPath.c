#include "globals.h"
#include "format.h"
#include "numberHelpers.h"
#include "exportPrivateKey.h"

// This class allows for the export of a number of very specific private keys. These private keys
// are made exportable as they are used in computations that are not feasible to carry out on the
// Ledger device. The key derivation paths that are allowed are restricted so that it is not
// possible to export keys that are used for signing.
static const uint32_t HARDENED_OFFSET = 0x80000000;
static exportPrivateKeyContext_t *ctx = &global.exportPrivateKeyContext;

int editDerivationPathPerKeyType(uint32_t *derivationPath,
                                 uint8_t derivationPathLength,
                                 uint8_t derivationPathKeyType,
                                 uint32_t account) {
    if (derivationPathLength > MAX_DERIVATION_PATH_LENGTH) {
        return 0;
    }
    switch (derivationPathKeyType) {
        case NEW_ID_CRED_SEC:
        case NEW_PRF_KEY:
        case NEW_SIGNATURE_BLINDING_RANDOMNESS:
            derivationPath[derivationPathLength++] = derivationPathKeyType | HARDENED_OFFSET;
            return derivationPathLength;
        case NEW_COMMITMENT_RANDOMNESS:
            derivationPath[derivationPathLength++] = derivationPathKeyType | HARDENED_OFFSET;
            derivationPath[derivationPathLength++] = account | HARDENED_OFFSET;
            return derivationPathLength;
        default:
            PRINTF("Invalid derivation path key type: %d\n", derivationPathKeyType);
            return 0;
    }
}

int exportNewPathPrivateKeysForPurpose(uint8_t purpose,
                                       uint32_t identityProvider,
                                       uint32_t identity,
                                       uint32_t account,
                                       uint8_t *outputPrivateKey,
                                       size_t outputPrivateKeySize) {
    uint32_t derivationPath[MAX_DERIVATION_PATH_LENGTH];
    uint8_t derivationPathLength = 4;
    cx_ecfp_private_key_t tempPrivateKeyEd25519;
    uint8_t tempPrivateKey[COMMON_PRIVATE_KEY_SIZE];

    uint8_t keysToExport[MAX_KEYS_TO_EXPORT] = {0, 0, 0};
    uint8_t keysToExportLength = 0;

    // Set the derivation path
    derivationPath[0] = NEW_PURPOSE | HARDENED_OFFSET;
    derivationPath[1] = NEW_COIN_TYPE | HARDENED_OFFSET;
    derivationPath[2] = identityProvider | HARDENED_OFFSET;
    derivationPath[3] = identity | HARDENED_OFFSET;

    switch (purpose) {
        case P1_IDENTITY_CREDENTIAL_CREATION:
            keysToExport[keysToExportLength++] = NEW_ID_CRED_SEC;
            keysToExport[keysToExportLength++] = NEW_PRF_KEY;
            keysToExport[keysToExportLength++] = NEW_SIGNATURE_BLINDING_RANDOMNESS;
            break;
        case P1_ACCOUNT_CREATION:
            keysToExport[keysToExportLength++] = NEW_PRF_KEY;
            keysToExport[keysToExportLength++] = NEW_ID_CRED_SEC;
            keysToExport[keysToExportLength++] = NEW_COMMITMENT_RANDOMNESS;
            break;
        case P1_ID_RECOVERY:
            keysToExport[keysToExportLength++] = NEW_ID_CRED_SEC;
            keysToExport[keysToExportLength++] = NEW_SIGNATURE_BLINDING_RANDOMNESS;
            break;
        case P1_ACCOUNT_CREDENTIAL_DISCOVERY:
            keysToExport[keysToExportLength++] = NEW_PRF_KEY;
            break;
        case P1_CREATION_OF_ZK_PROOF:
            keysToExport[keysToExportLength++] = NEW_COMMITMENT_RANDOMNESS;
            break;
        default:
            PRINTF("Invalid purpose: %d\n", purpose);
            THROW(ERROR_INVALID_PARAM);
    }

    // check if the buffer is big enough
    if (keysToExportLength * LENGTH_AND_PRIVATE_KEY_SIZE > outputPrivateKeySize) {
        PRINTF("There is not enough space for the keys in the output buffer\n");
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    uint8_t tx = 0;

    // iterate over the keys to export
    for (int keyIndex = 0; keyIndex < keysToExportLength; keyIndex++) {
        // Edit the derivation path according to the key to export
        uint8_t tempDeriviationPathLength = editDerivationPathPerKeyType(derivationPath,
                                                                         derivationPathLength,
                                                                         keysToExport[keyIndex],
                                                                         account);
        if (tempDeriviationPathLength == 0) {
            PRINTF("The erivation path length is too long\n");
            THROW(ERROR_BUFFER_OVERFLOW);
        }

        outputPrivateKey[tx++] = 32;  // length of the private key
        if (keysToExport[keyIndex] == NEW_COMMITMENT_RANDOMNESS) {
            // export raw key
            getPrivateKey(derivationPath, tempDeriviationPathLength, &tempPrivateKeyEd25519);
            for (int i = 0; i < 32; i++) {
                tempPrivateKey[i] = tempPrivateKeyEd25519.d[i];
            }
        } else {
            // export bls key
            getBlsPrivateKey(derivationPath,
                             tempDeriviationPathLength,
                             tempPrivateKey,
                             sizeof(tempPrivateKey));
        }

        for (int i = 0; i < 32; i++) {
            outputPrivateKey[tx++] = tempPrivateKey[i];
        }
    }
    explicit_bzero(&tempPrivateKey, sizeof(tempPrivateKey));
    explicit_bzero(&tempPrivateKeyEd25519, sizeof(tempPrivateKeyEd25519));
    return tx;
}

void handleExportPrivateKeyNewPath(uint8_t *dataBuffer,
                                   uint8_t p1,
                                   uint8_t lc,
                                   volatile unsigned int *flags) {
    if ((p1 != P1_IDENTITY_CREDENTIAL_CREATION && p1 != P1_ACCOUNT_CREATION &&
         p1 != P1_ID_RECOVERY && p1 != P1_ACCOUNT_CREDENTIAL_DISCOVERY &&
         p1 != P1_CREATION_OF_ZK_PROOF)) {
        THROW(ERROR_INVALID_PARAM);
    }

    size_t offset = 0;
    uint8_t remainingDataLength = lc;

    ////// Extract the identity provider //////
    if (remainingDataLength < 4) {
        THROW(ERROR_INVALID_PATH);
    }
    uint32_t identityProvider = U4BE(dataBuffer, offset);
    offset += 4;
    remainingDataLength -= 4;

    ////// Extract the identity //////
    if (remainingDataLength < 4) {
        THROW(ERROR_INVALID_PATH);
    }
    uint32_t identity = U4BE(dataBuffer, offset);
    offset += 4;
    remainingDataLength -= 4;

    ////// Extract the account //////
    uint32_t account = 0xFFFFFFFF;
    if (p1 == P1_ACCOUNT_CREATION || p1 == P1_CREATION_OF_ZK_PROOF) {
        if (remainingDataLength < 4) {
            THROW(ERROR_INVALID_PATH);
        }
        account = U4BE(dataBuffer, offset);
    }

    ctx->privateKeysLength = exportNewPathPrivateKeysForPurpose(p1,
                                                                identityProvider,
                                                                identity,
                                                                account,
                                                                ctx->outputPrivateKeys,
                                                                sizeof(ctx->outputPrivateKeys));
    ////// Set up the display //////
    offset = 0;
    /// Add the identity provider to the display
    memmove(ctx->display, "IDP#", 4);
    offset += 4;
    offset += bin2dec(ctx->display + offset, sizeof(ctx->display) - offset, identityProvider);
    /// Add the identity to the display
    // Remove the null terminator from the display to add the identity
    offset -= 1;
    memmove(ctx->display + offset, " ID#", 4);
    offset += 4;
    offset += bin2dec(ctx->display + offset, sizeof(ctx->display) - offset, identity);

    if (p1 == P1_IDENTITY_CREDENTIAL_CREATION) {
        /// Set the display header
        memmove(ctx->displayHeader, "ID Credential Creation", 23);
    } else if (p1 == P1_ACCOUNT_CREATION) {
        /// Set the display header
        memmove(ctx->displayHeader, "Account Creation", 17);
        /// Add the account to the display
        // Remove the null terminator from the display to add the account
        offset -= 1;
        memmove(ctx->display + offset, " ACCOUNT#", 9);
        offset += 9;
        bin2dec(ctx->display + offset, sizeof(ctx->display) - offset, account);
    } else if (p1 == P1_ID_RECOVERY) {
        /// Set the display header
        memmove(ctx->displayHeader, "ID Recovery", 12);
    } else if (p1 == P1_ACCOUNT_CREDENTIAL_DISCOVERY) {
        /// Set the display header
        memmove(ctx->displayHeader, "Account Cred Discovery", 23);
    } else if (p1 == P1_CREATION_OF_ZK_PROOF) {
        /// Set the display header
        memmove(ctx->displayHeader, "ZK Proof Creation", 18);
        /// Add the account to the display
        // Remove the null terminator from the display to add the account
        offset -= 1;
        memmove(ctx->display + offset, " ACCOUNT#", 9);
        offset += 9;
        bin2dec(ctx->display + offset, sizeof(ctx->display) - offset, account);
    }

    uiExportPrivateKeysNewPath(flags);
}

void sendPrivateKeysNewPath(void) {
    if ((size_t)ctx->privateKeysLength > sizeof(G_io_apdu_buffer)) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    memmove(G_io_apdu_buffer, ctx->outputPrivateKeys, ctx->privateKeysLength);
    sendSuccess(ctx->privateKeysLength);
    explicit_bzero(ctx->outputPrivateKeys, sizeof(ctx->outputPrivateKeys));
    ctx->privateKeysLength = 0;
}
