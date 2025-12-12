#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_DERIVATION_PATH_LENGTH 6
#define MAX_KEYS_TO_EXPORT         3

#define LENGTH_AND_PRIVATE_KEY_SIZE 33  // 1 byte for length, 32 bytes for private key

//// LEGACY PATHS ////
#define ACCOUNT_SUBTREE 0
#define NORMAL_ACCOUNTS 0
// Export the PRF key
#define P1_LEGACY_PRF_KEY          0x00
#define P1_LEGACY_PRF_KEY_RECOVERY 0x01
// Export the PRF key and the IdCredSec
#define P1_LEGACY_PRF_KEY_AND_ID_CRED_SEC 0x02
// Export seeds (Deprecated)
#define P2_LEGACY_SEED 0x01
// Export the BLS keys
#define P2_LEGACY_KEY 0x02

//// NEW PATHS ////
#define P1_IDENTITY_CREDENTIAL_CREATION 0x00
#define P1_ACCOUNT_CREATION             0x01
#define P1_ID_RECOVERY                  0x02
#define P1_ACCOUNT_CREDENTIAL_DISCOVERY 0x03
#define P1_CREATION_OF_ZK_PROOF         0x04

/**
 * Handles the export of private keys that are allowed to leave the device.
 * The export paths are restricted so that the method cannot access any account paths.
 * @param p1 has to be 0x00 for export of PRF key for decryption, 0x01 for export of PRF key for
 * recovering credentials and 0x02 for export of PRF key and IdCredSec.
 * @param p2 If set to 0x01, then the seeds are exported (Using this is deprecated). If set to 0x02,
 * then the BLS keys are exported. 0x00 is not used to ensure that old clients fail when calling
 * this functionality.
 */
void handleExportPrivateKeyLegacyPath(uint8_t *dataBuffer,
                                      uint8_t p1,
                                      uint8_t p2,
                                      uint8_t lc,
                                      volatile unsigned int *flags);

void handleExportPrivateKeyNewPath(uint8_t *dataBuffer,
                                   uint8_t p1,
                                   uint8_t lc,
                                   volatile unsigned int *flags);

typedef struct {
    uint8_t displayHeader[31];
    uint8_t display[40];
    bool exportBoth;
    bool exportSeed;
    uint32_t path[7];
    uint8_t pathLength;
    bool isNewPath;
    uint8_t outputPrivateKeys[MAX_KEYS_TO_EXPORT * LENGTH_AND_PRIVATE_KEY_SIZE];
    uint8_t privateKeysLength;
} exportPrivateKeyContext_t;

void uiExportPrivateKey(volatile unsigned int *flags);
void uiExportPrivateKeysNewPath(volatile unsigned int *flags);
void exportPrivateKey(void);
void sendPrivateKeysNewPath(void);
